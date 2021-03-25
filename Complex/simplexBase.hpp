#pragma once
#include <map>
#include <algorithm>
#include <set>
#include <memory>
#include <iostream>
#include <unordered_map>
#include "utils.hpp"

// Header file for simplexBase class - see simplexTree.cpp for descriptions


class simplexBase {
  private:
  public:
	std::vector<std::set<simplexNode_P, cmpByWeight>> simplexList;		//Holds ordered list of simplices in each dimension
																//Needs to sort by the weight for insertion

	unsigned simplexOffset = 0;

	long long nodeCount = 0;					//Total number of nodes stored
	long long indexCounter;						//Current insertion index

	utils ut;									//Utilities functions
	std::string simplexType = "simplexBase";	//Complex Type Identifier

	simplexNode_P root;							//Root of the simplexNode tree (if applicable)
	simplexNode_P head;							//Root of the simplexNode tree (if applicable)

	double maxEpsilon;							//Maximum epsilon, loaded from configuration
	int maxDimension;							//Maximum dimension, loaded from configuration
	std::vector<std::vector<double>>* distMatrix;	//Pointer to distance matrix for current complex


	//For sliding window implementation, tracks the current vectors inserted into the window
	//		Note - these point to the d0 simplexNodes; index, weight, etc. can be obtained
	std::vector<int> runningVectorIndices;   // std::vector<simplexNode*> runningVectorIndices;
	int runningVectorCount = 0;		//How many total points have been inserted into complex?
									//		Is this different than indexCounter?

	int removedSimplices = 0;
	std::string stats = "RVIndex,Mean,Stdev,k,kNN_Mean,kNN_Stdev,Result\n";

	//Constructors
	simplexBase();
	simplexBase(std::map<std::string, std::string>&);
	simplexBase(double, int);

	//Configurations of the complex
	void setConfig(std::map<std::string, std::string>&);
	void setDistanceMatrix(std::vector<std::vector<double>>* _distMatrix);
	void setEnclosingRadius(double);
	static simplexBase* newSimplex(const std::string &, std::map<std::string, std::string>&);

	//Stream evaluator - this uses a function to determine if points should be inserted into the complex
	bool (*streamEval) (std::vector<double>&, std::vector<std::vector<double>>&);
    bool streamEvaluator(std::vector<double>&, std::vector<std::vector<double>>&);
	void setStreamEvaluator(bool (*f) (std::vector<double>&, std::vector<std::vector<double>>&));

	//virtual interface functions
	virtual double getSize();
	virtual bool insertIterative(std::vector<double>&, std::vector<std::vector<double>>&);
	virtual bool insertIterative(std::vector<double>&, std::vector<std::vector<double>>&, int&, int&);
	virtual void deleteIterative(int);
	virtual void deleteIndexRecurse(int);  				// A wrapper for the actual deleteIndexRecurse method.
	virtual void insert();
	virtual bool find(std::vector<unsigned>);
	virtual bool find(std::set<unsigned>);
	virtual int simplexCount();
	virtual int vertexCount();
	virtual void prepareCofacets(int);
	virtual void prepareFacets(int);

	virtual std::vector<simplexNode_P> getAllCofacets(const std::set<unsigned>&, double, const std::unordered_map<simplexNode_P, simplexNode_P>&, bool);
	virtual std::vector<simplexNode*> getAllCofacets(simplexNode_P, const std::unordered_map<long long, simplexNode_P>&, bool);
	virtual std::vector<simplexNode*> getAllCofacets(simplexNode_P);
	virtual std::vector<simplexNode_P> getAllDelaunayCofacets(simplexNode_P);
	virtual std::vector<simplexNode_P> getAllCofacets(const std::set<unsigned>&);
	virtual std::vector<simplexNode*> getAllFacets(simplexNode*);
	virtual std::vector<simplexNode*> getAllFacets(simplexNode_P);
	virtual std::vector<simplexNode_P> getAllFacets_P(simplexNode_P);


	virtual std::set<simplexNode_P, cmpByWeight> getDimEdges(int);
	virtual std::vector<std::set<simplexNode_P, cmpByWeight>> getAllEdges();
	virtual std::vector<simplexNode_P> expandDimension(std::vector<simplexNode_P> edges);
  virtual void buildAlphaComplex(std::vector<std::vector<int>> dsimplexmesh, int npts,std::vector<std::vector<double>> inputData);
	virtual void expandDimensions(int);
	virtual void reduceComplex();
	virtual ~simplexBase();
	virtual void outputComplex();
};
