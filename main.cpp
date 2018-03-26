#include <omp.h>
#include <iostream>
#include<vector>
#include<sstream>
#include<algorithm>
#include <string>
#include<fstream>
#include <iterator>
using namespace std;

/*
 * Split Function
 * reference: https://stackoverflow.com/a/236803/5721245 
 */
template<typename Out>
void split(const string &s, char delim, Out result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		*(result++) = item;
	}
}

vector<string> split(const string &s, char delim) {
	std::vector<string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}
//---------

//new type to save data
struct dataFromFile
{
	vector < vector<float> > points;
	int numofpoints;
	int numofclus;
};

/*
 * retrieveData Function Declaration
 * retrieveData(string filePath)
 * return data as dataFromFile variable
 */
dataFromFile retrieveData(string filePath)
{
	int numOfPoints, numOfClusters;
	vector<vector<float> > dataFloat; //vector of points
	ifstream dataFile(filePath);
	if (dataFile.is_open()){
		string line; // to store each line of file
		int counter = 0; // to ignore the first line of the file
		while (!dataFile.eof()) // end of file not true
		{
			getline(dataFile, line); // get the current line of file 'dataFile' and stores it to variable 'line'
			if (counter == 0) // first line checker
			{
				vector<string> x = split(line, ' ');
				istringstream(x[0]) >> numOfPoints; // number of points = first number 
				istringstream(x[1]) >> numOfClusters; // number of clusters = second number
			}
			else
			{
				float x1, x2, x3, x4;
				vector<string> x = split(line, ',');
				if (x.size() == 4){
					istringstream(x[0]) >> x1; //first number = first point and ...etc
					istringstream(x[1]) >> x2;
					istringstream(x[2]) >> x3;
					istringstream(x[3]) >> x4;
					dataFloat.push_back({ x1, x2, x3, x4 }); // add this points to dataFloat vector 
				}
			}
			counter++;
		}
	}
	dataFromFile DataByeBye;
	DataByeBye.points = dataFloat;
	DataByeBye.numofpoints = numOfPoints;
	DataByeBye.numofclus = numOfClusters;

	dataFile.close();
	return DataByeBye;
}

/*
 * Random Cluster Generator Function Declaration
 */
vector<vector<float> > randomClus(int numofclus, vector < vector<float> > dataFloat)
{
	vector<vector<float> > Kvec;
	for (int i = 0; i < numofclus; i++)
	{
		Kvec.push_back(dataFloat[rand() % dataFloat.size()]); // generate random int from 0 to 149
	}
	return Kvec;
}

/*
* Random Cluster Generator Function Declaration
*/
vector<vector<float> > parRandomClus(int numofclus, vector < vector<float> > dataFloat)
{
	vector<vector<float> > Kvec;
	int id;
	#pragma omp for private(id) ordered
		for (int i = 0; i < numofclus; i++)
		{
			Kvec.push_back(dataFloat[rand() % dataFloat.size()]); // generate random int from 0 to 149
		}
	
	return Kvec;
}


/*
* Euclidian Function Declaration
* euclidian(vector of cluster, vector of points, numOfClusters)
* save data in pointToCluster variable
*/
vector<vector<int> > euclidian(vector<vector<float> > Kvec, vector < vector<float> > dataFloat, int numOfClusters)
{
	vector<vector<int> > pointToCluster; //vector of clusters assigned to nearest points
	for (int i = 0; i < numOfClusters; i++) pointToCluster.push_back({}); //intialization the vector of number of clusters

	/*
	* Euclidian
	* this loop loops through the points and assign for each cluster indx, the indx of points which nearest distance
	*/
	for (int i = 0; i< dataFloat.size(); i++)
	{
		float minDis = INT_MAX;
		int minPIndx, minCIndx;
		for (int j = 0; j < numOfClusters; j++)
		{
			//The Equation of Euclidian
			float min = sqrt(pow(dataFloat[i][0] - Kvec[j][0], 2) + pow(dataFloat[i][1] - Kvec[j][1], 2) + pow(dataFloat[i][2] - Kvec[j][2], 2) + pow(dataFloat[i][3] - Kvec[j][3], 2));
			//-------------------------

			if (min < minDis)
			{
				minPIndx = i;
				minCIndx = j;
				minDis = min;
			}
		}

		//pointToCluster[clusterindx].push_back(pointindx)
		pointToCluster[minCIndx].push_back(minPIndx);
		//--------------------
	}
	//-------------

	return pointToCluster;
}

/*
* Euclidian Function Declaration
* euclidian(vector of cluster, vector of points, numOfClusters)
* save data in pointToCluster variable
*/
vector<vector<int> > parEuclidian(vector<vector<float> > Kvec, vector < vector<float> > dataFloat, int numOfClusters)
{
	vector<vector<int> > pointToCluster; //vector of clusters assigned to nearest points
	for (int i = 0; i < numOfClusters; i++) pointToCluster.push_back({}); //intialization the vector of number of clusters

	/*
	* Euclidian
	* this loop loops through the points and assign for each cluster indx, the indx of points which nearest distance
	*/
#pragma omp for ordered
	for (int i = 0; i< dataFloat.size(); i++)
	{
		float minDis = INT_MAX;
		int minPIndx, minCIndx;
		for (int j = 0; j < numOfClusters; j++)
		{
			//The Equation of Euclidian
			float min = sqrt(pow(dataFloat[i][0] - Kvec[j][0], 2) + pow(dataFloat[i][1] - Kvec[j][1], 2) + pow(dataFloat[i][2] - Kvec[j][2], 2) + pow(dataFloat[i][3] - Kvec[j][3], 2));
			//-------------------------

			if (min < minDis)
			{
				minPIndx = i;
				minCIndx = j;
				minDis = min;
			}
		}

		//pointToCluster[clusterindx].push_back(pointindx)
		pointToCluster[minCIndx].push_back(minPIndx);
		//--------------------
	}
	//-------------

	return pointToCluster;
}

int seqKmeans(string outputFilePath, string filePath, int threshold)
{
	cout << "Sequential Kmeans running...\n";
	/*
	* Read data from file
	* params are the file path
	* return is a dataFromFile variable
	*/
	dataFromFile dataRetrieved = retrieveData(filePath);
	vector < vector<float> > dataFloat = dataRetrieved.points;
	int numOfPoints = dataRetrieved.numofpoints, numOfClusters = dataRetrieved.numofclus;
	//----------------

	/*
	* generating random clusters
	* params are number of clusters and the points
	* return is a vector of vector of points
	*/
	vector<vector<float> > Kvec = randomClus(numOfClusters, dataFloat);
	//----------------

	while (true) // stops when the test Covergence achieved
	{
		vector<vector<int> > pointToCluster; //vector of clusters assigned to nearest points

		/*
		* Euclidian Function Call
		* euclidian(vector of cluster, vector of points, numOfClusters)
		* save data in pointToCluster variable
		*/
		pointToCluster = euclidian(Kvec, dataFloat, numOfClusters);
		//------------

		float p0 = 0, p1 = 0, p2 = 0, p3 = 0; //p stands for point, num stands for indx of point

		/*
		* converge to count the number of clusters with same centroid =Test 1=
		* converge2 to count the number of clusters verify the thresold =Test 2=
		*/
		int converge = 0, converge2 = 0;
		//----------------------

		/*
		* this loop generate new centroids
		* loop through clusters and sum each points then divides them by the size of this cluster
		*/

		for (int indx = 0; indx < numOfClusters; indx++)
		{
			p0 = 0, p1 = 0, p2 = 0, p3 = 0;
			int points = pointToCluster[indx].size();

			//loop to sum
			for (int innerindx = 0; innerindx < points; innerindx++)
			{
				p0 += dataFloat[pointToCluster[indx][innerindx]][0];
				p1 += dataFloat[pointToCluster[indx][innerindx]][1];
				p2 += dataFloat[pointToCluster[indx][innerindx]][2];
				p3 += dataFloat[pointToCluster[indx][innerindx]][3];
			}
			//----------

			// division
			p0 = p0 / points;
			p1 = p1 / points;
			p2 = p2 / points;
			p3 = p3 / points;
			//---------- 

			//Convergence Test 1
			if (p0 == Kvec[indx][0] && p1 == Kvec[indx][1] && p2 == Kvec[indx][2] && p3 == Kvec[indx][3])
				converge++;

			//Convergence Test 2
			else if (roundf(p0 * 1000) / 1000 - roundf(Kvec[indx][0] * 1000) / 1000 == threshold && roundf(p1 * 1000) / 1000 - roundf(Kvec[indx][1] * 1000) / 1000 == threshold && roundf(p2 * 1000) / 1000 - roundf(Kvec[indx][2] * 1000) / 1000 == threshold && roundf(p3 * 1000) / 1000 - roundf(Kvec[indx][3] * 1000) / 1000 == threshold)
				converge2++;

			Kvec[indx] = { p0, p1, p2, p3 }; //assign new points
		}

		// Convergence Verification
		if (converge>1 || converge2>1)
		{
			cout << "running Finished!\n";
			//c_str() to convert string to char *
			freopen(outputFilePath.c_str(), "w", stdout);
			// if error 'freopen deprecated' appears in visual studio, follow this steps to ignore
			// https://stackoverflow.com/a/46034465/5721245

			for (int indx = 0; indx < Kvec.size(); indx++)
			{
				cout << "Cluster(" << indx << "): ";
				cout << roundf(Kvec[indx][0] * 1000) / 1000 << ' ' << roundf(Kvec[indx][1] * 1000) / 1000 << ' ' << roundf(Kvec[indx][2] * 1000) / 1000 << ' ' << roundf(Kvec[indx][3] * 1000) / 1000 << endl;
			}
			fclose(stdout);
			return 0;
		}
	}
}

int parKmeans(string outputFilePath, string filePath, int threshold)
{
	cout << "Parallel Kmeans running...\n";
	/*
	* Read data from file
	* params are the file path
	* return is a dataFromFile variable
	*/
	dataFromFile dataRetrieved = retrieveData(filePath);
	vector < vector<float> > dataFloat = dataRetrieved.points;
	int numOfPoints = dataRetrieved.numofpoints, numOfClusters = dataRetrieved.numofclus;
	//----------------

	/*
	* generating random clusters
	* params are number of clusters and the points
	* return is a vector of vector of points
	*/
	vector<vector<float> > Kvec = parRandomClus(numOfClusters, dataFloat);
	//----------------

	while (true) // stops when the test Covergence achieved
	{
		vector<vector<int> > pointToCluster; //vector of clusters assigned to nearest points

		/*
		* Euclidian Function Call
		* euclidian(vector of cluster, vector of points, numOfClusters)
		* save data in pointToCluster variable
		*/
		pointToCluster = parEuclidian(Kvec, dataFloat, numOfClusters);
		//------------

		float p0 = 0, p1 = 0, p2 = 0, p3 = 0; //p stands for point, num stands for indx of point

		/*
		* converge to count the number of clusters with same centroid =Test 1=
		* converge2 to count the number of clusters verify the thresold =Test 2=
		*/
		int converge = 0, converge2 = 0;
		//----------------------

		/*
		* this loop generate new centroids
		* loop through clusters and sum each points then divides them by the size of this cluster
		*/

		for (int indx = 0; indx < numOfClusters; indx++)
		{
			p0 = 0, p1 = 0, p2 = 0, p3 = 0;
			int points = pointToCluster[indx].size();
#pragma omp parallel
			{
				//loop to sum
#pragma omp for reduction(+:p0) reduction( +:p1) reduction( +:p2) reduction( +:p3)
				for (int innerindx = 0; innerindx < points; innerindx++)
				{
					p0 += dataFloat[pointToCluster[indx][innerindx]][0];
					p1 += dataFloat[pointToCluster[indx][innerindx]][1];
					p2 += dataFloat[pointToCluster[indx][innerindx]][2];
					p3 += dataFloat[pointToCluster[indx][innerindx]][3];
				}
			}
			//----------

			// division
			p0 = p0 / points;
			p1 = p1 / points;
			p2 = p2 / points;
			p3 = p3 / points;
			//---------- 

			//Convergence Test 1
			if (p0 == Kvec[indx][0] && p1 == Kvec[indx][1] && p2 == Kvec[indx][2] && p3 == Kvec[indx][3])
				converge++;

			//Convergence Test 2
			else if (roundf(p0 * 1000) / 1000 - roundf(Kvec[indx][0] * 1000) / 1000 == threshold && roundf(p1 * 1000) / 1000 - roundf(Kvec[indx][1] * 1000) / 1000 == threshold && roundf(p2 * 1000) / 1000 - roundf(Kvec[indx][2] * 1000) / 1000 == threshold && roundf(p3 * 1000) / 1000 - roundf(Kvec[indx][3] * 1000) / 1000 == threshold)
				converge2++;

			Kvec[indx] = { p0, p1, p2, p3 }; //assign new points
		}

		// Convergence Verification
		if (converge>1 || converge2>1)
		{
			cout << "running Finished!\n";
			//c_str() to convert string to char *
			freopen(outputFilePath.c_str(), "w", stdout);
			// if error 'freopen deprecated' appears in visual studio, follow this steps to ignore
			// https://stackoverflow.com/a/46034465/5721245
			for (int indx = 0; indx < Kvec.size(); indx++)
			{
				cout << "Cluster(" << indx << "): ";
				cout << roundf(Kvec[indx][0] * 1000) / 1000 << ' ' << roundf(Kvec[indx][1] * 1000) / 1000 << ' ' << roundf(Kvec[indx][2] * 1000) / 1000 << ' ' << roundf(Kvec[indx][3] * 1000) / 1000 << endl;
			}
			fclose(stdout);
			return 0;
		}
	}
}

int main()
{
	const float threshold = floorf(.001 * 1000) / 1000; //constant threshold, you can change it from here once
	const string filePath = "IrisDataset.txt"; //path of the file, you can change it from here once
	const string outputFilePath = "IrisDataset_cluster_centres.txt"; //path of the output file, you can change it from here once

	int choice;
	cout << "1. Sequential K-means algorithm\n2. Parallel K - Means using OMP\n\n	Your Choice: ";
	cin >> choice;

	choice == 1 ? seqKmeans(outputFilePath, filePath, threshold) : choice == 2 ? parKmeans(outputFilePath, filePath, threshold) : printf("\n\nYou selected wrong choice ");
	
	return 0;
}
