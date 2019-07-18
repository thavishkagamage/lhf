/*
 * basePipe hpp + cpp protoype and define a base class for building
 * pipeline functions to execute
 *
 */
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <chrono>
#include <string>
#include <numeric>
#include <iostream>
#include <functional> 
#include <vector>
#include "densityUtils.hpp"
#include "utils.hpp"
//////// DBSCAN algorithm for standalone clustering or as an initialization step for DenStream /////////

// basePipe constructor
densityUtils::densityUtils(){
	procName = "densityUtils";
    return;
}
//taking in preprocessor type


// runPipe -> Run the configured functions of this pipeline segment
pipePacket densityUtils::runPreprocessor(pipePacket inData){    //standalone preprocessor
/////////constants//////////
 utils ut;
 double epsilon = 0.5; //epsilon (radius)- how close points should be to constitute a cluster (need to adjust but preferably small)
 int minPoints = 4; //(density threshold) - minimum # of points to from a dense region (# dimensions +1, (+ >1) ifdata/larger data)

 std::vector<int> processed(inData.workData.originalData.size(), 0); 
 std::vector<int> noiseClassifier(inData.workData.originalData.size(), 0);  // 1 = noise
 std::vector<int> clusterLabel(inData.workData.originalData.size(), 0);  //cluster each non noise Point ends up in
 std::vector<int> neighborTracker;
////////////  DBSCAN Revisited, Revisited Schubert 2017 ///////////
 int clusterIndex = 0;

for(int i = 0; i<inData.workData.originalData.size(); i++){  //for each point
    if(processed[i] == 0){ //if point has not been processed or clustered already 
       processed[i] == 1;
       std::vector<std::vector<double>> neighbors = neighborQuery(inData.workData.originalData, neighborTracker, i, epsilon);  //  find initial neighbors of current point
       if(neighbors.size() < minPoints){    // --non core points are marked as noise relative to current point
            noiseClassifier[i] = 1;
       }
       else{
           clusterIndex +=1;
           if( expandCluster(inData.workData.originalData, neighbors, neighborTracker, clusterLabel, clusterIndex, epsilon, minPoints, processed)){
              clusterLabel[i] = clusterIndex;
          }
       }
    }
}

///// find centroids of clusters so pass to next pipe

	return inData;
}

std::vector<int> densityUtils::dbscan(std::vector<std::vector<double>>& data){   //initialization for DenStream
utils ut; 

 double epsilon = 0.5; //epsilon (radius)- how close points should be to constitute a cluster (need to adjust but preferably small)
 int minPoints = 4; //(density threshold) - minimum # of points to from a dense region (# dimensions +1, (+ >1) ifdata/larger data)

 std::vector<int> processed(data.size(), 0); 
 std::vector<int> noiseClassifier(data.size(), 0);  // 1 = noise
 std::vector<int> clusterLabel(data.size(), 0);  //cluster each non noise Point ends up in
 // std::vector<int> neighborLabel(inData.workData.originalData.size(), 0);
std::vector<int> neighborTracker;
////////////  DBSCAN Revisited, Revisited Schubert 2017 ///////////
 int clusterIndex = 0;

for(int i = 0; i<data.size(); i++){  //for each point
    if(processed[i] == 0){ //if point has not been processed already 
       processed[i] == 1;
       std::vector<std::vector<double>> neighbors = neighborQuery(data, neighborTracker, i, epsilon);  //  -find initial neighbors of current point
       if(neighbors.size() < minPoints){    // --non core points are marked as noise
            noiseClassifier[i] = 1;
       }
       else{
           clusterIndex +=1;
           if( expandCluster(data, neighbors, neighborTracker, clusterLabel, clusterIndex, epsilon, minPoints, processed)){
              clusterLabel[i] = clusterIndex;
          }
       }
    }
} 
//will return all clusters found so that these clusters can either be pruned or maintained by DenStream


return clusterLabel;

}


std::vector<std::vector<double>> densityUtils::neighborQuery(std::vector<std::vector<double>>& data, std::vector<int>& neighborTracker, int i, double epsilon) {  //range query
   utils ut;
   std::vector<std::vector<double>> neighbors;
   int index = 1;
 
   for(int j = 0; j<data.size(); j++){   //check all points to see if they are within epsilon of query point i
      if(ut.vectors_distance(data[i], data[j] ) <= epsilon) {
         neighbors.push_back(data[j]); 
         neighborTracker.push_back(j);  // val @ neighborTracker[0] = index of first neighbor in OG data set
      }
   }
    return neighbors;    //return all neighbor points of current point
}

 int densityUtils::expandCluster(std::vector<std::vector<double>>& data, std::vector<std::vector<double>>& neighbors, std::vector<int>& neighborTracker,  std::vector<int>& clusterLabel, int clusterIndex, double epsilon, int minPoints, std::vector<int>& processed){
     //query all neighbor points of the current query point to determine if they are core (assigned to cluster) or border
       for(int i = 0; i<neighbors.size(); i++){  // for points within epsilon of query point, determine if they are core or border
        //  visited[i] = 1;
          std::vector<std::vector<double>>  neighborPrime = neighborQuery(data, neighborTracker, i, epsilon); //assigns core points to neighborPrime vector
          if (neighborPrime.size() > minPoints){// assign core points to cluster
             clusterLabel[neighborTracker[i]] = clusterIndex; 
             processed[neighborTracker[i]] = 1; //mark point as processed
          }
          //else point is a border point for this cluster and is up for grabs for future clusters
       }

   return 0;


 }


// configPipe -> configure the function settings of this pipeline segment
bool densityUtils::configPreprocessor(std::map<std::string, std::string> configMap){
  /*  auto preprocessor = configMap.find("clusters");
     if(preprocessor !=configMap.end())
        num_clusters = std::atoi(configMap["clusters"].c_str());
    else return false;

    preprocessor = configMap.find("iterations");
	if(preprocessor != configMap.end())
		num_iterations = std::atoi(configMap["iterations"].c_str());
	else return false;  */

	return true;
}
