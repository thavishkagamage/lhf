/*
 * basePipe hpp + cpp protoype and define a base class for building
 * pipeline functions to execute  
 * 
 */

#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "preprocessor.hpp"
#include "kMeansPlusPlus.hpp"
#include "streamingKmeans.hpp"
#include "denStream.hpp"

// basePipe constructor
preprocessor::preprocessor(){
	return;
}

preprocessor* preprocessor::newPreprocessor(const std::string &procName){
	utils ut;
	ut.writeDebug("preprocessor","Building preprocessor: " + procName);

	if(procName == "none"){
		return new preprocessor();
	} else if (procName == "kmeansplusplus" || procName == "kmeans++" || procName == "kmeans"){
		return new kMeansPlusPlus();
	} else if(procName == "streamingKmeans" || procName == "streamingkmeans" || procName =="streamKM"){
		return new streamingKmeans();
	} else if(procName == "denStream" || procName == "denstream" || procName =="DenStream"){
		return new denStream();
	} 

	return 0;
}

// runPipeWrapper -> wrapper for timing of runPipe and other misc. functions
void preprocessor::runPreprocessorWrapper(pipePacket &inData){
	
	//Check if the preprocessor has been configured
	if(!configured){
		ut.writeLog(procName,"Pipe not configured");
		std::cout << "Pipe not configured" << std::endl;
		return;
	}
	
	if(debug){
		
		//Start a timer for physical time passed during the pipe's function
		auto startTime = std::chrono::high_resolution_clock::now();
		
		runPreprocessor(inData);
		
		//Stop the timer for time passed during the pipe's function
		auto endTime = std::chrono::high_resolution_clock::now();
		
		//Calculate the duration (physical time) for the pipe's function
		std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
		
		//Output the time and memory used for this pipeline segment
		std::cout << "\tPipeline " << procName << " executed in " << (elapsed.count()/1000.0) << " seconds (physical time)" << std::endl << std::endl;
		
		auto dataSize = inData.getSize();
		auto unit = "B";
		
		std::cout << "Test" << std::endl;
		//Convert large datatypes (GB, MB, KB)
		if(dataSize > 1000000000){
			//Convert to GB
			dataSize = dataSize/1000000000;
			unit = "GB";
		} else if(dataSize > 1000000){
			//Convert to MB
			dataSize = dataSize/1000000;
			unit = "MB";
		} else if (dataSize > 1000){
			//Convert to KB
			dataSize = dataSize/1000;
			unit = "KB";
		}
		
		inData.stats += procName + "," + std::to_string(elapsed.count()/1000.0) + "," + std::to_string(dataSize) + "," + unit + "\n";
		
		outputData(inData);
	
	} else {
		runPreprocessor(inData);
	}
}

void preprocessor::outputData(std::vector<unsigned> data){
	std::ofstream file;
	file.open("output/" + procName + "_label_output.csv");
	
	for (auto a : data){
		file << std::to_string(a) << "\n";
	}
	file.close();
	return;
}
	
void preprocessor::outputData(pipePacket &data){
	
	outputData(data.workData);
	outputData(data.centroidLabels);
	
	return;
	
	
}

// outputData -> used for tracking each stage of the pipeline's data output without runtime
void preprocessor::outputData(std::vector<std::vector<double>> data){
	std::ofstream file;
	file.open("output/" + procName + "_centroid_output.csv");
	
	for (auto a : data){
		for (auto d : a){
			file << std::to_string(d) << ",";
		}
		file << "\n";
	}
	
	file.close();
	return;
}
	
// runPipe -> Run the configured functions of this pipeline segment
void preprocessor::runPreprocessor(pipePacket &inData){
	
	std::cout << "No run function defined for: " << procName << std::endl;
	
	return;
}	

// configPipe -> configure the function settings of this pipeline segment
bool preprocessor::configPreprocessor(std::map<std::string, std::string> &configMap){
	std::cout << "No configure function defined for: " << procName << std::endl;
	
	return false;
}

preprocessor::~preprocessor(){}