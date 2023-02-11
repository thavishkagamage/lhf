#pragma once

#include "basePipe.hpp"
#include "utils.hpp"

// basePipe constructor
template <typename nodeType> 
class incrementalPipe : public basePipe<nodeType> {
  private:
    std::vector<std::vector<double>> inputData;
    std::vector<unsigned> search_space;
    std::vector<std::vector<double>>* distMatrix;

  public:
    double circumRadius(std::set<unsigned>,unsigned);
    int expand_d_minus_1_simplex(std::set<unsigned>, unsigned);
    incrementalPipe();
    void runPipe(pipePacket<nodeType>& inData);
    bool configPipe(std::map<std::string, std::string> &configMap);
    void outputData(pipePacket<nodeType>&);
    // std::vector<std::vector<int>> qdelaunay_o(const delaunay &delaunay);
};
