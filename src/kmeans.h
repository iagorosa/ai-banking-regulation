#ifndef KMEANS
#define KMEANS

#include <math.h>
#include <vector>
#include <iostream>

class ClusterMethod
{
private:
	double **mpSample;//input
	double **mpCenters;//Storage Cluster Centers
	double **pDistances;//Distance Matrix
	int mSampleNum;//Number of samples
	int mClusterNum;//Number of Clusters
	int mFeatureNum;//Number of features per sample
	int *ClusterResult;//Clustering results
	int MaxIterationTimes;//Maximum number of iterations

public:
	void GetClusterd(double** features, int ClusterNum, int SampleNum, int FeatureNum);//External Interface
    double** GetCentroids();
    void testCall();
	double** createdata(int datanum, int featurenum);


private:
	void Initialize(double** features, int ClusterNum, int SampleNum, int FeatureNum);//Class Initialization
	void k_means();//Algorithm entry
	void k_means_Initialize();//Membership Matrix Initialization
	void k_means_Calculate();//Cluster calculation
};

#endif // KMEANS
