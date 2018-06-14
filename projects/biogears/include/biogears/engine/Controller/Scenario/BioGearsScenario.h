/**************************************************************************************
Copyright 2015 Applied Research Associates, Inc.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the License
at:
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
**************************************************************************************/

#pragma once
#include <biogears/cdm/scenario/SEScenario.h>
#include <biogears/engine/Controller/Scenario/BioGearsScenarioInitialParameters.h>

/**
* @brief A BioGears specific scenario (i.e. holds a %BioGears configuration object)
*/
class BIOGEARS_API BioGearsScenario : public SEScenario {
public:
  BioGearsScenario(SESubstanceManager& subMgr);
  virtual ~BioGearsScenario();

  virtual BioGearsScenarioInitialParameters& GetInitialParameters();
  virtual const BioGearsScenarioInitialParameters* GetInitialParameters() const;
};