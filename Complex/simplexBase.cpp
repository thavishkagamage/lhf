#include <string>
#include <vector>
#include <cmath>
#include <numeric>
#include <typeinfo>
#include "simplexBase.hpp"
#include "simplexTree.hpp"
#include "simplexArrayList.hpp"

simplexBase::simplexBase(){return;}

simplexBase::simplexBase(std::map<std::string, std::string> configMap){
	setConfig(configMap);

	return;
}

simplexBase::simplexBase(double maxE, int maxDim){
	maxEpsilon = maxE;
	maxDimension = maxDim;
}

void simplexBase::setConfig(std::map<std::string, std::string> configMap){
	std::string debug;
	std::string outputFile;

	auto pipe = configMap.find("debug");
	if(pipe != configMap.end())
		debug = std::atoi(configMap["debug"].c_str());
	pipe = configMap.find("outputFile");
	if(pipe != configMap.end())
		outputFile = configMap["outputFile"].c_str();
	pipe = configMap.find("dimensions");
	if(pipe != configMap.end())
		maxDimension = std::atoi(configMap["dimensions"].c_str());
	else return;
	pipe = configMap.find("epsilon");
	if(pipe != configMap.end())
		maxEpsilon = std::atof(configMap["epsilon"].c_str());
	else return;

	ut = utils(debug, outputFile);
	ut.writeLog(simplexType,"Configured utils for : " + simplexType);

	return;
}


void simplexBase::setDistanceMatrix(std::vector<std::vector<double>>* _distMatrix){
	distMatrix = _distMatrix;
	return;
}

// simplexTree constructor, currently no needed information for the class constructor
simplexBase* simplexBase::newSimplex(const std::string &simplexT, std::map<std::string, std::string> configMap){
	simplexType = simplexT;

	if(simplexType == "simplexTree"){
		auto t = new simplexTree(maxEpsilon, distMatrix, maxDimension);
		t->setConfig(configMap);
		return t;
	} else if (simplexType == "simplexArrayList"){
		auto t = new simplexArrayList(maxEpsilon, maxDimension, distMatrix);
		t->setConfig(configMap);
		return t;
	}
	return 0;
}


std::set<simplexNode*, cmpByWeight> simplexBase::getDimEdges(int dim){
	if(dim >= simplexList.size()){
		ut.writeLog(simplexType,"Error: requested dimension beyond complex");
		std::set<simplexNode*, cmpByWeight> a;
		return a;
	}
	return simplexList[dim];
}

std::vector<std::set<simplexNode*, cmpByWeight>> simplexBase::getAllEdges(){
	return simplexList;
}

std::vector<simplexNode*> simplexBase::getAllCofacets(const std::set<unsigned>& simplex){
	return getAllCofacets(simplex, 0, std::unordered_map<simplexNode*, simplexNode*>(), false);
}

std::vector<simplexNode*> simplexBase::getAllCofacets(const std::set<unsigned>& simplex, double simplexWeight, const std::unordered_map<simplexNode*, simplexNode*>& pivotPairs, bool checkEmergent){
	ut.writeLog(simplexType,"No get cofacets function defined");
	std::vector<simplexNode*> ret;
	return ret;
}

double simplexBase::getSize(){
	ut.writeLog(simplexType,"No size function defined");
	return -1;
}

bool simplexBase::insertIterative(std::vector<double>&, std::vector<std::vector<double>>&, int&, int&){
	ut.writeLog(simplexType,"No insert iterative function defined");
	return false;
}

bool simplexBase::insertIterative(std::vector<double>&, std::vector<std::vector<double>>&){
	ut.writeLog(simplexType,"No insert iterative function defined");
	return false;
}


void simplexBase::deleteIterative(int){
	ut.writeLog(simplexType,"No delete iterative function defined");
	return;
}


void simplexBase::deleteIndexRecurse(int){
	ut.writeLog(simplexType,"No recursive delete function defined");
	return;
}


void simplexBase::insert(){
	ut.writeLog(simplexType,"No insert function defined");
	return;
}

bool simplexBase::find(std::vector<unsigned>){
	ut.writeLog(simplexType,"No find function defined");
	return false;
}

bool simplexBase::find(std::set<unsigned>){
	ut.writeLog(simplexType,"No find function defined");
	return false;
}

int simplexBase::vertexCount(){
	ut.writeLog(simplexType,"No vertexCount function defined");
	return -1;
}

void simplexBase::prepareCofacets(int dim){
	ut.writeLog(simplexType,"No prepareCofacets function defined");
	return;
}

int simplexBase::simplexCount(){
	ut.writeLog(simplexType,"No simplexCount function defined");
	return -1;
}

void simplexBase::outputComplex(){
	ut.writeLog(simplexType,"No outputComplex function defined");
	return;
}

void simplexBase::expandDimensions(int dim){
	ut.writeLog(simplexType,"No expandDimensions function defined");
	return;
}

void simplexBase::reduceComplex(){
	ut.writeLog(simplexType,"No reduceComplex function defined");
	return;
}

void simplexBase::setStreamEvaluator(bool (*f) (std::vector<double>&, std::vector<std::vector<double>>&)){
	streamEval = f;
	ut.writeLog(simplexType,"Changed stream evaluator");
	return;
}


bool simplexBase::streamEvaluator(std::vector<double>& vector, std::vector<std::vector<double>>& window){
	//Do some evaluation of whether the point should stay or not
	//		For now, let's look at the deviation of connections

	auto reps = ut.nearestNeighbors(vector, window);

	double sum = std::accumulate(reps.begin(), reps.end(), 0.0);
	double mean = sum / reps.size();

	std::vector<double> diff(reps.size());
	std::transform(reps.begin(), reps.end(), diff.begin(),std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / reps.size());

	stats += std::to_string(runningVectorCount) + "," + std::to_string(mean) + "," + std::to_string(stdev) + ",";

	std::sort(reps.begin(), reps.end());
	std::vector<double> kNN;
	int k = 20;

	for(int i = 0; i < k; i++){
		kNN.push_back(reps[i]);
	}

	double sum_NN = std::accumulate(kNN.begin(), kNN.end(), 0.0);
	double mean_NN = sum_NN / kNN.size();

	std::vector<double> diff_NN(kNN.size());
	std::transform(kNN.begin(), kNN.end(), diff_NN.begin(),std::bind2nd(std::minus<double>(), mean_NN));
	double sq_sum_NN = std::inner_product(diff_NN.begin(), diff_NN.end(), diff_NN.begin(), 0.0);
	double stdev_NN = std::sqrt(sq_sum_NN / kNN.size());

	stats += std::to_string(k) + "," + std::to_string(mean_NN) + "," + std::to_string(stdev_NN) + ",";

	if (true){//stdev_NN > 10000){
		//std::cout << "\tAccept: (stdev > 0.5 , " << stdev << ")" << std::endl;
		stats += "Accept\n";
		return true;
	}
	stats += "Reject\n";
	//std::cout << "\tReject: (stdev > 0.5 , " << stdev << ")" << std::endl;
	return false;
}

void simplexBase::clear(){
	ut.writeLog(simplexType,"No clear function defined");
	return;
}
