/*
 * AnalysisEDA.cpp
 *
 * This file contains the implementation of the simulator.
 */

#include "AnalysisEDA.h"
#include "Circuit/Circuit.h"
#include <iostream>
#include "Library/TerminalInfo.h"

std::vector<NetWithValue> netValues;
std::vector<ElementChange> elements;

/*
* This function calculates the output of an element by checking what kind of element it is and doing the appropriate logic.
* @input: element to be calculated
*
*/
void AnalysisEDA::calculateOutput(ElementChange* elementChange){
  
    // get all input values
    std::vector<NetWithValue> inputValues;
    
    for (NetWithValue& netValue : netValues) {
        const Net* net = netValue.net;
        for (const Element* outElement : net->getOutElements()) {
            if (outElement == elementChange->element) {
                inputValues.push_back(netValue);
                break;
            }
        }
    }

    // calculate output
    const ElementType elementType = elementChange->element->getElementInfo()->getType();

    switch (elementType) {
        case ElementType::And:
            elementChange->value = Logic::logic1;
            for (NetWithValue inputNet : inputValues) {
                if (inputNet.value == Logic::logic0) {
                    elementChange->value = Logic::logic0;
                    break;
                }
                else if (inputNet.value == Logic::logicX) {
                    if (elementChange->value == Logic::logic1) {
                        elementChange->value = Logic::logicX;
                    }
                }
            }
            break;
        case ElementType::Or:
            elementChange->value = Logic::logic0;
            for (NetWithValue inputNet : inputValues) {
                if (inputNet.value == Logic::logic1) {
                    elementChange->value = Logic::logic1;
                    break;
                }
                else if (inputNet.value == Logic::logicX) {
                    if (elementChange->value == Logic::logic0) {
                        elementChange->value = Logic::logicX;
                    }
                }
                else {
                    continue;
                }
            }
            break;
        case ElementType::Not:
            if (inputValues[0].value == Logic::logic0) {
                elementChange->value = Logic::logic1;
            }
            else if (inputValues[0].value == Logic::logic1) {
                elementChange->value = Logic::logic0;
            }
            break;
        case ElementType::Dff:
            if (inputValues[0].value == Logic::logic1) {
                elementChange->value = inputValues[1].value;
            }
            break;
        default:
            break;
    }

    // set output
    for (NetWithValue& netValue : netValues) {
        if (netValue.net->getInElement() == elementChange->element) {
            if (netValue.value == elementChange->value) {
                continue;
            }
            
            netValue.value = elementChange->value;
            
            for (const Element* element : netValue.net->getOutElements()) {
                for (ElementChange& elementChange : elements) {
                    if (elementChange.element == element) {
                        elementChange.hasChanges = true;
                        break;
                    }
                }
            }
            break;
        }
    }
}


void AnalysisEDA::run() {
    // list of all elements
    std::vector<int> primaryOutputIndexes;

    for (const Element* element : circuit->getAllElements()) {
        elements.push_back(ElementChange(element));
    }

    for(const Net* net : circuit->getAllNets()) {
        netValues.push_back(NetWithValue(net, Logic::logicX));
    }

    // sort netValues by net name to set input values in the correct order
    std::sort(netValues.begin(), netValues.end(), [](NetWithValue& a, NetWithValue& b) {
        // sort by net name, make exception for clock
        if (a.net->getName() == "CLOCK") {
            if (b.net->getName().begin()[2] == '0' && b.net->getName().begin()[1] == '0'){
                return false;
            }
            return true;
        }
        else if (b.net->getName() == "CLOCK") {
            if (a.net->getName().begin()[2] == '0' && a.net->getName().begin()[1] == '0'){
                return true;
            }
            return false;
        }
        return a.net->getName() < b.net->getName();
    });

    // find primary outputs
    int index = 0;
    for (NetWithValue& netValue : netValues) {
        if (netValue.net->getOutElements()[0] == nullptr) {
            primaryOutputIndexes.push_back(index);
        }
        index++;
    }

    // Iterate all time steps:
    for (const std::vector<Logic>& timeStep : inputData) {
        // set input values
        int index = 0;
        for (const Logic& value : timeStep) {
            int count = 0;
            for (NetWithValue& netValue : netValues) {
                if (count == index){
                    netValue.value = value;

                    for (const Element* element : netValue.net->getOutElements()) {
                        for (ElementChange& elementChange : elements) {
                            if (elementChange.element == element) {
                                elementChange.hasChanges = true;
                                
                            }
                        }
                    }
                }
                count++;
                if (count > index){
                    break;
                }
            }
            index++;
        }


        // Iterate all elements:
        bool hasChanges = true;
        bool first = true;
        while (hasChanges){
            hasChanges = false;
            if (first) {
                first = false;
            
                for (ElementChange& elementChange : elements) {
                    if (elementChange.element->getElementInfo()->getType() == ElementType::Dff) {
                        elementChange.hasChanges = false;
                        this->calculateOutput(&elementChange);
                        hasChanges = true;
                    }
                }
            } else{
                for (ElementChange& elementChange : elements) {
                    if (elementChange.hasChanges && elementChange.element->getElementInfo()->getType() == ElementType::Dff) {
                        elementChange.hasChanges = false;
                    }
                }
            }

            for (ElementChange& elementChange : elements) {
                if (elementChange.hasChanges && elementChange.element->getElementInfo()->getType() != ElementType::Dff) {
                    elementChange.hasChanges = false;
                    this->calculateOutput(&elementChange);
                    hasChanges = true;
                }
            }
        }

        // print primary output
        first = true;
        for (int primaryOutputIndex : primaryOutputIndexes) {
            if(first){
                std::cout << netValues[primaryOutputIndex].value;
                first = false;
            }else{
                std::cout << ";" << netValues[primaryOutputIndex].value;
            }
        }
        std::cout << std::endl;

    }
}

