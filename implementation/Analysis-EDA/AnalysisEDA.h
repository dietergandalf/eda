/*
 * AnalysisEDA.h
 */

#ifndef AnalysisEDA_H_
#define AnalysisEDA_H_

#include <vector>
#include "template.h"
#include "Circuit/Element.h"
#include "Circuit/Net.h"
#include "Library/TerminalInfo.h"


// forward declarations
class Circuit;

class NetWithValue {
public:
    NetWithValue(const Net* net, Logic value) : net(net), value(value) {}
    const Net* net;
    Logic value;
};

class ElementChange {
public:
    ElementChange(const Element* element) : element(element) {}
    const Element* element;
    Logic value = Logic::logicX;
    bool hasChanges = false;
};


class AnalysisEDA {
public:
    AnalysisEDA(const Circuit* circuit, const std::vector<std::vector<Logic>>& inputData) :
        circuit(circuit), inputData(inputData) {
    }
    virtual ~AnalysisEDA() {
    }

    void run();
    void calculateOutput(ElementChange* element);


private:
    // prevent from using the default constructor, copy constructor and assignment operator
    AnalysisEDA();
    AnalysisEDA(const AnalysisEDA&);
    AnalysisEDA& operator=(const AnalysisEDA&);

    const Circuit* circuit;
    const std::vector<std::vector<Logic>>& inputData;
};

#endif /* AnalysisEDA_H_ */
