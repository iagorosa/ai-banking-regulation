//param@v saves the classification result v[i][j][k] to represent the kth feature of the jth data in class I (starting from 0)
//param@feateres Input data feateres[i][j] represents the jth feature of the ith data (i, J starts from 0)
//Number of param@ClusterNum categories
//Number of param@SampleNum data
//param@FeatureNum Data Feature Number

#include "kmeans.h"


using namespace std;


void ClusterMethod::GetClusterd(double** features, int ClusterNum, int SampleNum, int FeatureNum)
{
	Initialize(features, ClusterNum, SampleNum, FeatureNum);
	k_means();
}
/*
void ClusterMethod::GetCentroids()
{
	return mpCenters
}*/

void ClusterMethod::testCall()
{
	cout << "foi\n" << endl;
}


//Intra-class data initialization
void ClusterMethod::Initialize(double** features, int ClusterNum, int SampleNum, int FeatureNum)
{
	mpSample = features;
	mFeatureNum = FeatureNum;
	mSampleNum = SampleNum;
	mClusterNum = ClusterNum;
	MaxIterationTimes = 50;

	mpCenters = new double*[mClusterNum];
	for (int i = 0; i < mClusterNum; ++i)
	{
		mpCenters[i] = new double[mFeatureNum];
	}

	pDistances = new double*[mSampleNum];
	for (int i = 0; i < mSampleNum; ++i)
	{
		pDistances[i] = new double[mClusterNum];
	}

	ClusterResult = new int[mSampleNum];
}


//Algorithm entry
void ClusterMethod::k_means()
{
	k_means_Initialize();
	k_means_Calculate();
}


//Initialize Cluster Center
void ClusterMethod::k_means_Initialize()
{
	for (int i = 0; i < mClusterNum; ++i)
	{
		//mpCenters[i] = mpSample[i];

		for (int k = 0; k < mFeatureNum; ++k)
		{
			mpCenters[i][k] = mpSample[i][k];
		}
	}
}

//Clustering process
void ClusterMethod::k_means_Calculate()
{
	double DBL_MAX = 2000.0;

	double J = DBL_MAX;//objective function
	int time = MaxIterationTimes;

	while (time)

	{
		double now_J = 0;//Target function after last update of distance Center
		--time;
                
                //Distance Initialization
		for (int i = 0; i < mSampleNum; ++i)
		{
			for (int j = 0; j < mClusterNum; ++j)
			{
				pDistances[i][j] = 0;

			}
		}
                //Calculate Euclidean Distance
		for (int i = 0; i < mSampleNum; ++i)
		{
			for (int j = 0; j < mClusterNum; ++j)
			{
				for (int k = 0; k < mFeatureNum; ++k)
				{
					pDistances[i][j] += abs(pow(mpSample[i][k], 2) - pow(mpCenters[j][k], 2));
				}
				now_J += pDistances[i][j];
			}
		}
	
		if (J - now_J < 0.01)//The objective function stops changing and ends the loop
		{	
			break;
		}
		J = now_J;

                //a Stores temporary classification results
		vector<vector<vector<double> > > a(mClusterNum);
		for (int i = 0; i < mSampleNum; ++i)
		{
			
			double min = DBL_MAX;
			for (int j = 0; j < mClusterNum; ++j)
			{
				if (pDistances[i][j] < min)
				{
					min = pDistances[i][j];
					ClusterResult[i] = j;
				}
			}

			vector<double> vec(mFeatureNum);
			for (int k = 0; k < mFeatureNum; ++k)
			{
				vec[k] = mpSample[i][k];
			}
			a[ClusterResult[i]].push_back(vec);
		//	v[ClusterResult[i]].push_back(vec); this cannot be done here because v has no initialization size
		}
		// v = a;

		//Calculating New Cluster Centers
		for (int j = 0; j < mClusterNum; ++j)
		{
			for (int k = 0; k < mFeatureNum; ++k)
			{

				mpCenters[j][k] = 0;
			}
		}


		for (int j = 0; j < mClusterNum; ++j)
		{
			for (int k = 0; k < mFeatureNum; ++k)
			{
				for (int s = 0; s < a[j].size(); ++s)
				{
					mpCenters[j][k] += a[j][s][k];
				}
				if (a[j].size() != 0)
				{
					mpCenters[j][k] /= a[j].size();
				}
			}
		}
	}

        //Output Cluster Center
	for (int j = 0; j < mClusterNum; ++j)
	{
		for (int k = 0; k < mFeatureNum; ++k)
		{
			cout << mpCenters[j][k] << " ";
		}
		cout << endl;
	}
}

//Number of param@datanum data
//param@featurenum number of features per data
double** ClusterMethod::createdata(int datanum, int featurenum)
{
	srand((int)time(0));
	double** data = new  double*[datanum];
	for (int i = 0; i < datanum; ++i)
	{
		data[i] = new double[featurenum];
	}
	cout << "Input data:" << endl;
	for (int i = 0; i < datanum ; ++i)
	{
		for (int j = 0; j < featurenum; ++j)
		{
			data[i][j] = ((int)rand() % 30) / 10.0;
			cout << data[i][j] << " ";
		}
		cout << endl;
	}

	return data;
}
