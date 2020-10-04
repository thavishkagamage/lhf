/*
 * persistencePairs hpp + cpp extend the basePipe class for calculating the
 * persistence pairs numbers from a complex
 *
 */

#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <exception>
#include <unordered_map>
#include <queue>
#include "incrementalPersistence.hpp"
#include "simplexArrayList.hpp"
#include "utils.hpp"

// basePipe constructor
incrementalPersistence::incrementalPersistence(){
	pipeType = "IncrementalPersistence";
	return;
}

// runPipe -> Run the configured functions of this pipeline segment
//
//	IncrementalPersistence: For computing the persistence pairs from simplicial complex:
//		1. See Bauer-19 for algorithm/description
void incrementalPersistence::runPipe(pipePacket &inData){
	simplexArrayList* complex;

	if(inData.complex->simplexType == "simplexArrayList"){
		complex = (simplexArrayList*) inData.complex;
	} else{
		std::cout<<"IncrementalPersistence does not support complexes other than simplexArrayList\n";
		return;
	}

	//Get all edges for the simplexArrayList
	std::vector<std::set<simplexNode_P, cmpByWeight>> edges = complex->getAllEdges();

	if(edges.size() <= 1) return;

	//Some notes on fast persistence:

	//	-Vectors need to be stored in a lexicograhically ordered set of decreasing (d+1)-tuples (e.g. {3, 1, 0})
	//		-These vectors are replaced with their indices (e.g. {{2,1,0} = 0, {3,1,0} = 1, {3,2,0} = 2, etc.})

	//	-Boundary matrix stores reduced collection of cofaces for each column (indexed)

	//Start a timer for physical time passed during the pipe's function
	auto startTime = std::chrono::high_resolution_clock::now();

	//Get all dim 0 persistence intervals
	//Kruskal's minimum spanning tree algorithm
	//		Track the current connected components (uf)
	//		Check edges; if contained in a component, ignore
	//			if joins components, add them
	//		Until all edges evaluated or MST found (size - 1)

	//For streaming data, indices will not be 0-N; instead sparse
	//	So in streaming, create a hash map to quickly lookup points
	//TODO - POSSIBLY FIX
	std::vector<simplexNode_P> pivots; //Store identified pivots
	unsigned mstSize = 0;
	unsigned nPts = inData.originalData.size();

	unionFind uf(nPts);

	for(auto edgeIter = edges[1].begin(); edgeIter != edges[1].end(); edgeIter++){
		std::set<unsigned>::iterator it = (*edgeIter)->simplex.begin();

		//Find which connected component each vertex belongs to
		//	Use a hash map to track insertions for streaming or sparse indices
		unsigned v1 = *it;
		int c1 = uf.find(v1);

		it++;
		unsigned v2 = *it;
		int c2 = uf.find(v2);

		//Edge connects two different components -> add to the MST
		if(c1 != c2){
			uf.join(c1, c2);
			mstSize++;

			simplexNode_P temp = std::make_shared<simplexNode>(simplexNode((*edgeIter)->simplex, (*edgeIter)->weight));
			temp->hash = v1 + v2*(v2-1)/2;
			pivots.push_back(temp);

			bettiBoundaryTableEntry des = { 0, 0, (*edgeIter)->weight, temp->simplex };
			inData.bettiTable.push_back(des);
		}

		//Check if we've filled our MST and can break
		if(mstSize >= edges[0].size()-1) break;
	}

	// std::cout << "mappedIndices.size = " << mappedIndices.size() << '\n';

	for(int i=0; i<inData.originalData.size(); i++){
		if(uf.find(i) == i){ //i is the name of a connected component
			//Each connected component has an open persistence interval
			bettiBoundaryTableEntry des = { 0, 0, maxEpsilon, {} };
			inData.bettiTable.push_back(des);
		}
	}

	//For higher dimensional persistence intervals
	//
		//Build next dimension of ordered simplices, ignoring previous dimension pivots

		//Represent the ordered simplices as indexed sets

		//Identify apparent pairs - i.e. d is the youngest face of d+1, and d+1 is the oldest coface of d
		//		This indicates a feature represents a trivial persistence interval

		//Track V (reduction matrix) for each column j that has been reduced to identify the constituent
		//		boundary simplices

	for(unsigned d = 1; d < dim && d < edges.size()-1; d++){
		std::sort(pivots.begin(), pivots.end(), cmpBySecond());
		std::vector<simplexNode_P>::iterator it = pivots.begin();

		std::vector<simplexNode_P> nextPivots;	 					//Pivots for the next dimension
		std::unordered_map<simplexNode_P, std::vector<simplexNode_P>> v;				//Store only the reduction matrix V and compute R implicity
		std::unordered_map<long long, simplexNode_P> pivotPairs;	//For each pivot, which column has that pivot

		//Iterate over columns to reduce in reverse order
		for(auto columnIndexIter = edges[d].rbegin(); columnIndexIter != edges[d].rend(); columnIndexIter++){
			simplexNode_P simplex = (*columnIndexIter);		//The current simplex

			//Not a pivot -> need to reduce
			if((*it)->hash != simplex->hash){
				//Get all cofacets using emergent pair optimization
				std::vector<simplexNode*> cofaceList = complex->getAllCofacets(simplex, pivotPairs);
				std::vector<simplexNode_P> columnV;	//Reduction column of matrix V
				columnV.push_back(simplex); //Initially V=I -> 1's along diagonal

				//Build a heap using the coface list to reduce and store in V
				std::make_heap(cofaceList.begin(), cofaceList.end(), cmpBySecond());

				while(true){
					simplexNode* pivot;
					while(!cofaceList.empty()){
						pivot = cofaceList.front();

						//Rotate the heap
						std::pop_heap(cofaceList.begin(), cofaceList.end(), cmpBySecond());
						cofaceList.pop_back();

						if(!cofaceList.empty() && pivot->hash == cofaceList.front()->hash){ //Coface is in twice -> evaluates to 0 mod 2
							delete pivot;
							delete cofaceList.front();

							//Rotate the heap
							std::pop_heap(cofaceList.begin(), cofaceList.end(), cmpBySecond());
							cofaceList.pop_back();
						} else{

							cofaceList.push_back(pivot);
							std::push_heap(cofaceList.begin(), cofaceList.end(), cmpBySecond());
							break;
						}
					}

					if(cofaceList.empty()){ //Column completely reduced
						break;
					} else if(pivotPairs.find(pivot->hash) == pivotPairs.end()){ //Column cannot be reduced
						pivotPairs.insert({pivot->hash, simplex});
						nextPivots.push_back(std::shared_ptr<simplexNode>(pivot));

						std::sort(columnV.begin(), columnV.end());
						auto it = columnV.begin();
						while(it != columnV.end()){
							if((it+1) != columnV.end() && *it==*(it+1)) ++it;
							else v[simplex].push_back(*it);
							++it;
						}

						for(int i=1; i<cofaceList.size(); i++) delete cofaceList[i];

						if(simplex->weight != pivot->weight){
							bettiBoundaryTableEntry des = { d, simplex->weight, pivot->weight, ut.extractBoundaryPoints(v[simplex]) };
							inData.bettiTable.push_back(des);
						}

						break;
					} else{ //Reduce the column of R by computing the appropriate columns of D by enumerating cofacets
						for(simplexNode_P simp : v[pivotPairs[pivot->hash]]){
							columnV.push_back(simp);
							std::vector<simplexNode*> cofaces = complex->getAllCofacets(simp);
							cofaceList.insert(cofaceList.end(), cofaces.begin(), cofaces.end());
						}
						std::make_heap(cofaceList.begin(), cofaceList.end(), cmpBySecond());
					}
				}

			//Was a pivot, skip the evaluation and queue next pivot
			} else ++it;
		}

		pivots = nextPivots;
	}

	//Stop the timer for time passed during the pipe's function
	auto endTime = std::chrono::high_resolution_clock::now();

	//Calculate the duration (physical time) for the pipe's function
	std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

	//Output the time and memory used for this pipeline segment
	ut.writeDebug("persistence","Bettis executed in " + std::to_string(elapsed.count()/1000.0) + " seconds (physical time)");;

	return;
}



// outputData -> used for tracking each stage of the pipeline's data output without runtime
void incrementalPersistence::outputData(pipePacket &inData){
	std::ofstream file;
	if(fnmod.size() > 0)
		file.open("output/"+pipeType+"_bettis_output"+fnmod+".csv");
	else
		file.open("output/" + pipeType + "_bettis_output.csv");

	for(auto row : inData.bettiTable)
		file << std::to_string(row.bettiDim) << "," << std::to_string(row.birth) << "," << std::to_string(row.death) << std::endl;
	
	file.close();

	file.close();

	file.open("output/tArray.csv");

	file << "Dim,Birth,Death,Simplex\n";
	for(auto tStruct : inData.bettiTable){
		file << tStruct.bettiDim << "," << tStruct.birth << "," << tStruct.death << ",";
		for(auto index : tStruct.boundaryPoints)
			file << index << " ";
		file << "\n";
	}
	file.close();

	return;
}


// configPipe -> configure the function settings of this pipeline segment
bool incrementalPersistence::configPipe(std::map<std::string, std::string> &configMap){
	std::string strDebug;

	auto pipe = configMap.find("debug");
	if(pipe != configMap.end()){
		debug = std::atoi(configMap["debug"].c_str());
		strDebug = configMap["debug"];
	}
	pipe = configMap.find("outputFile");
	if(pipe != configMap.end())
		outputFile = configMap["outputFile"].c_str();

	ut = utils(strDebug, outputFile);

	pipe = configMap.find("dimensions");
	if(pipe != configMap.end())
		dim = std::atoi(configMap["dimensions"].c_str());
	else return false;

	pipe = configMap.find("epsilon");
	if(pipe != configMap.end())
		maxEpsilon = std::atof(configMap["epsilon"].c_str());
	else return false;

	pipe = configMap.find("fn");
	if(pipe != configMap.end())
		fnmod = configMap["fn"];

	configured = true;
	ut.writeDebug("incrementalPersistence","Configured with parameters { dim: " + configMap["dimensions"] + ", complexType: " + configMap["complexType"] + ", eps: " + configMap["epsilon"]);
	ut.writeDebug("incrementalPersistence","\t\t\t\tdebug: " + strDebug + ", outputFile: " + outputFile + " }");

	return true;
}
