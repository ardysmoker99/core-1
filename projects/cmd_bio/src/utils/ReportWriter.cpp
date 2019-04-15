#include "ReportWriter.h"
#include "Tokenizer.h"
#include "string-helpers.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <numeric>
#include <iomanip>

namespace biogears {

TableRow::TableRow() {}
TableRow::TableRow(std::string field_n, std::string expected_v, double engine_v, std::string percent_e, std::string n)
{
  field_name = field_n;
  expected_value = expected_v;
  engine_value = engine_v;
  percent_error = percent_e;
  notes = n;
  passed = true;
}
TableRow::~TableRow() {}

ReferenceValue::ReferenceValue() {}
ReferenceValue::~ReferenceValue() {}

ReportWriter::ReportWriter() {}
ReportWriter::~ReportWriter() {}

void ReportWriter::gen_tables_single_sheet(const char* validation_file, const char* baseline_file)
{
  gen_tables_single_sheet(std::string(validation_file), std::string(baseline_file));
}

void ReportWriter::gen_tables_single_sheet(std::string validation_file, std::string baseline_file)
{
  ParseReferenceCSV(validation_file);
  ParseBaselineCSV(baseline_file);
  CalculateAverages();
  ExtractValues();
  Validate();
  PopulateTables();
  to_markdown();
  clear();
}

void ReportWriter::gen_tables()
{
  std::vector<std::string> validation_files{ "BloodChemistryValidation.csv",
                                             "CardiovascularValidation.csv",
                                             "EnergyValidation.csv",
                                             "EndocrineValidation.csv",
                                             "RenalValidation.csv",
                                             "TissueValidation.csv" };
  std::vector<std::string> baseline_files{ "BloodChemistryValidationResults.csv",
                                           "CardiovascularValidationResults.csv",
                                           "EnergyValidationResults.csv",
                                           "EndocrineValidationResults.csv",
                                           "RenalValidationResults.csv",
                                           "TissueValidationResults.csv" };
  std::vector<std::string> reports;
  for (int i = 0; i < validation_files.size(); i++) {
    ParseReferenceCSV(std::string(validation_files[i]));
    ParseBaselineCSV(std::string(baseline_files[i]));
    CalculateAverages();
    ExtractValues();
    Validate();
    PopulateTables();
    reports.push_back(to_html());
    clear();
  }
  return;
}

std::string ReportWriter::to_html()
{
  report.append(std::string("<html><body>\n"));
  //...Do work
  for (auto table_itr = tables.begin(); table_itr != tables.end(); ++table_itr) {
    std::string table;
    std::string table_name = table_itr->first;
    table += std::string("<table border=\"1\">\n");
    for (int i = 0; i < table_itr->second.size(); i++) {
      std::string line("<tr ");
      if (table_itr->second[i].passed) {
        line += (i==0) ? ">" : "bgcolor=#32CD32>";
      } else {
        line += (i==0) ? ">" : "bgcolor=#FF0000>";
      }
      line += "<td>";
      line += table_itr->second[i].field_name;
      line += "</td>";
      line += "<td>";
      line += table_itr->second[i].expected_value;
      line += "</td>";
      line += "<td>";
      line += (i == 0) ? "Engine Value" : std::to_string(table_itr->second[i].engine_value);
      line += "</td>";
      line += "<td>";
      line += table_itr->second[i].percent_error;
      line += "</td>";
      line += "<td>";
      line += table_itr->second[i].notes;
      line += "</td>";
      line += "</tr>\n";
      table.append(line);
    }
    table += std::string("</table>\n");
    // This block saves out the md tables for website generation
    std::ofstream html_file;
    html_file.open("validation/tables/" + table_name + "ValidationTable.html");
    if (!html_file) {
      return "Error writing html file";
    }
    html_file << (std::string("<html><body>\n") + table + std::string("\n</body></html>\n"));
    html_file.close();
    //
    report.append(table);
    table.clear();
  }
  report.append(std::string("</body></html>\n"));
  return report;
}

std::string ReportWriter::to_markdown()
{
  for (auto table_itr = tables.begin(); table_itr != tables.end(); ++table_itr) {
    std::string table;
    std::string table_name = table_itr->first;
    for (int i = 0; i < table_itr->second.size(); i++) {
      std::string line_prepend;
      std::string line_append("</span>");
      if (table_itr->second[i].passed) {
        line_prepend = "<span style=\"background-color: #32CD32\">";
      } else {
        line_prepend = "<span style=\"background-color: #FF0000\">";
      }
      std::string line("|");
      line += table_itr->second[i].field_name;
      line += "|";
      line += table_itr->second[i].expected_value;
      line += "|";
      line += (i == 0) ? "Engine Value" : std::to_string(table_itr->second[i].engine_value);
      line += "|";
      line += table_itr->second[i].percent_error;
      line += "|";
      line += table_itr->second[i].notes;
      line += "|\n";
      table.append(line);
      if (i == 0) { //So markdown needs this line after the first line to know that it's representing a table
        table.append("|---|---|---|---|---|\n");
      }
    }
    // This block saves out the md tables for website generation
    std::ofstream md_file;
    md_file.open("validation/tables/" + table_name + "ValidationTable.md");
    if (!md_file) {
      return "Error writing md file";
    }
    md_file << table;
    md_file.close();
    //
    report.append(table + "\n");
  }
  return report;
}

std::string ReportWriter::to_xml()
{
  report.append(std::string("<xml><body>\n"));
  //...Do work
  for (auto table_itr = tables.begin(); table_itr != tables.end(); ++table_itr) {
    report.append(std::string("<table border=\"1\">"));

    for (int i = 0; i < table_itr->second.size(); i++) {
      std::string line("<tr");
      if (table_itr->second[i].passed) {
        line += "bgcolor=#32CD32>";
      } else {
        line += "bgcolor=#FF0000>";
      }
      line += "<td>";
      line += table_itr->second[i].field_name;
      line += "</td>";
      line += "<td>";
      line += table_itr->second[i].expected_value;
      line += "</td>";
      line += "<td>";
      line += (i == 0) ? "Engine Value" : std::to_string(table_itr->second[i].engine_value);
      line += "</td>";
      line += table_itr->second[i].percent_error;
      line += "</td>";
      line += "<td>";
      line += table_itr->second[i].notes;
      line += "</td>";
      line += "</tr>";
      report.append(line);
    }
    report.append(std::string("</table>"));
  }

  //...Finish work
  report.append(std::string("</body></xml>\n"));
  return report;
}

void ReportWriter::ParseReferenceCSV(const char* filename)
{
  ParseReferenceCSV(std::string(filename));
}

void ReportWriter::ParseReferenceCSV(std::string filename)
{
  ParseCSV(filename, this->validation_data);
}

void ReportWriter::ParseBaselineCSV(const char* filename)
{
  ParseBaselineCSV(std::string(filename));
}

void ReportWriter::ParseBaselineCSV(std::string filename)
{
  ParseCSV(filename, this->biogears_results);
}

void ReportWriter::ParseCSV(std::string& filename, std::vector<std::vector<std::string>>& data)
{
  std::ifstream file{ filename };
  if (!file.is_open()) {
    return;
  }
  std::string line;
  int line_number = 0;
  int index = 0;
  while (std::getline(file, line)) {
    std::string cell;
    std::vector<std::string> vec;
    data.push_back(vec);
    for (int i = 0; i < line.size(); i++) {
      if (line[i] == ',') {
        data[line_number].push_back(ltrim(cell));
        cell.clear();
      } else if (line[i] == '"') {
        while (true) {
          ++i;
          if (i == line.size()) {
            std::string next_line;
            std::getline(file, next_line);
            line += next_line;
          }
          if (line[i] == '"') {
            break;
          } else {
            cell += line[i];
          }
        }
      } else {
        cell += line[i];
      }
    }
    data[line_number].push_back(ltrim(cell));
    cell.clear();
    ++line_number;
  }
}

void ReportWriter::CalculateAverages()
{
  std::vector<biogears::TableRow> rows;
  for (int i = 0; i < biogears_results[0].size(); i++) {
    biogears::TableRow row;
    row.field_name = biogears_results[0][i];
    row.expected_value = "0.0";
    rows.push_back(row);
  }
  for (int i = 1; i < biogears_results.size(); i++) {
    for (int k = 0; k < biogears_results[i].size(); k++) {
      rows[k].engine_value += std::stod(biogears_results[i][k]);
    }
  }
  for (int i = 0; i < rows.size(); i++) {
    rows[i].engine_value /= biogears_results.size() - 1;
    std::string field_name_with_units = rows[i].field_name;
    TableRow row = rows[i]; //So field_name_with_units looks like "Name(Unit)", which is why it gets split to just be "Name"
    table_row_map.insert(std::pair<std::string, biogears::TableRow>(split(field_name_with_units, '(')[0], row));
  }
}

void ReportWriter::ExtractValues()
{
  for (int i = 1; i < validation_data.size(); i++) {
    biogears::ReferenceValue ref;
    ref.value_name = validation_data[i][0];
    ref.unit_name = validation_data[i][1];
    if (validation_data[i][2][0] == '[') {
      ref.is_range = true; // In the case that the validation data looks like [num1,num2],....
      // This line splits [num1,num2],.... into a vector where the first element is num1,num2
      // Then it splits the first vector element into a vector where the first two elements are num1 and num2
      std::vector<std::string> value_range = split(split(validation_data[i][2].substr(1), ']')[0], ',');
      ref.reference_range = std::pair<double, double>(std::stod(value_range[0]), std::stod(value_range[1]));
    } else {
      ref.is_range = false;
      std::vector<std::string> value = split(validation_data[i][2], ',');
      ref.reference_value = std::stod(value[0]);
    }
    ref.reference = split(validation_data[i][3], ',')[0];
    ref.notes = validation_data[i][4];
    ref.table_name = validation_data[i][5];
    //!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/
    ref.error_threshold = 0.5; // This is a placeholder value
    reference_values.push_back(ref);
    //!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/
  }
}

void ReportWriter::Validate()
{
  for (biogears::ReferenceValue ref : reference_values) {
    auto table_row_itr = table_row_map.find(ref.value_name);
    if (table_row_itr == table_row_map.end()) {
      continue;
    }
    biogears::TableRow table_row = table_row_itr->second;
    if (ref.is_range) {
      table_row.expected_value = "[" + std::to_string(ref.reference_range.first) + "," + std::to_string(ref.reference_range.second) + "]" + "@" + ref.reference;
      if (ref.reference_range.first <= table_row.engine_value && ref.reference_range.second >= table_row.engine_value) {
        table_row.passed = true;
        table_row.percent_error = "Within Bounds";
      } else {
        table_row.passed = false;
        table_row.percent_error = "Outside Bounds";
      }
    } else {
      table_row.expected_value = std::to_string(ref.reference_value) + "@" + ref.reference;
      if(std::fabs(ref.reference_value - table_row.engine_value) <= 0.0001) {
        table_row.percent_error = "0.0%";
        table_row.passed = true;
      } else {
        double error = (std::fabs(ref.reference_value - table_row.engine_value)/((ref.reference_value + table_row.engine_value)/2));
        error = std::fabs(error);
        table_row.percent_error = std::to_string(error*100) + "%";
        if (ref.error_threshold >= error) {
          table_row.passed = true;
        } else {
          table_row.passed = false;
        }
      }
    }
    table_row.notes = ref.notes;
    table_row.table_name = ref.table_name;
    table_row_itr->second = table_row;
  }
}

void ReportWriter::PopulateTables()
{
  for (auto itr = table_row_map.begin(); itr != table_row_map.end(); ++itr) {
    auto table_itr = tables.find(itr->second.table_name);
    if (table_itr != tables.end()) {
      table_itr->second.push_back(itr->second);
    } else {
      biogears::TableRow tr(itr->second.table_name, "Expected Value", 0.0, "Percent Error", "Notes");
      std::vector<biogears::TableRow> tr_vec;
      tr_vec.push_back(tr);
      tables.insert(std::pair<std::string, std::vector<biogears::TableRow>>(itr->second.table_name, tr_vec));
      table_itr = tables.find(itr->second.table_name);
      table_itr->second.push_back(itr->second);
    }
  }
}

void ReportWriter::clear()
{
  tables.clear();
  table_row_map.clear();
  reference_values.clear();
  validation_data.clear();
  biogears_results.clear();
  report.clear();
}
}
