#include <pcl/ModelCoefficients.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/features/normal_3d.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/recognition/hv/hv_go.h>
#include <string>
#include <pcl/io/ply_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/surface/on_nurbs/triangulation.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/registration/icp.h>
#include <pcl/features/shot_omp.h>
#include <pcl/recognition/hv/greedy_verification.h>
// including opencv2 headers
#include <opencv2/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <pcl/filters/uniform_sampling.h>
#include <pcl/io/openni2_grabber.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/registration/sample_consensus_prerejective.h>
//#include <OpenNI.h>
#include <Eigen/Dense>
#include <thread>
// writing into a file
#include <ostream>
#include <vector>
#include <iterator>
#include <memory>
#include <iostream>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

//cpprestsdk
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/containerstream.h>


#include <cstdlib>
#include <codecvt>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams


using namespace std;
using namespace cv;
using namespace boost::filesystem;
using namespace openni;
typedef pcl::SHOT352 DescriptorType;


bool debug = false;
bool live = false;
bool doAlignment = false;
std::string projectSrcDir = PROJECT_SOURCE_DIR;


HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);//handling console color for shapenet shapes

int consoleColr = 0;

string locationForOutputClouds = projectSrcDir + "/data/temoclouds/";// location for temp out clouds passed for classification
// Read in the cloud data

string path2classifier = projectSrcDir + "/data/pointnet/";//where my fork of pointnet exists
auto classificationServerURL = L"http://192.168.178.58:5070/";
string scenesRGBDMainPath = projectSrcDir + "/data/test/";//where the folders of the test scenes exist
string mainModelsPath = projectSrcDir + "/data/models";//where the models exist

string teamOutput = projectSrcDir + "/data/teamoutput";//where the models exist


string dir_challenge = "";
string  color_scene_filename = "";
string  challenge_name = "";





pcl::PointCloud<pcl::PointXYZRGBA>::Ptr birdCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr houseCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr canCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr crackerCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr shoeCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);


pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedbirdCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedhouseCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedcanCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedcrackerCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedshoeCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);




pcl::PointCloud<pcl::Normal>::Ptr birdNormals(new pcl::PointCloud<pcl::Normal>);
pcl::PointCloud<pcl::Normal>::Ptr shoeNormals(new pcl::PointCloud<pcl::Normal>);

pcl::PointCloud<pcl::Normal>::Ptr canNormals(new pcl::PointCloud<pcl::Normal>);
pcl::PointCloud<pcl::Normal>::Ptr crackerNormals(new pcl::PointCloud<pcl::Normal>);
pcl::PointCloud<pcl::Normal>::Ptr houseNormals(new pcl::PointCloud<pcl::Normal>);


template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}


// Creates an HTTP request and prints part of its response stream.
pplx::task<string> HTTPStreamingAsync(const string ply)
{
	
	http_client client(classificationServerURL);
	http_request req(methods::POST);
	req.set_body(ply);
	//auto resp = client.request(req);
	//auto r = resp.get();

	return client.request(req).then([](http_response response)
	{
		/*if (response.status_code() != status_codes::OK)
		{
		// Handle error cases...
		return pplx::task_from_result();
		}*/

		// Perform actions here reading from the response stream... 
		// In this example, we print the first 15 characters of the response to the console.
		concurrency::streams::istream bodyStream = response.body();
		container_buffer<std::string> inStringBuffer;
		return bodyStream.read(inStringBuffer, 100).then([inStringBuffer](size_t bytesRead)
		{
			std::string &text
				= inStringBuffer.collection();
			//cout << text << endl;
			return text;
			// For demonstration, convert the response text to a wide-character string.
			//	std::wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t> utf16conv;
			//	std::wostringstream ss;
			//ss << utf16conv.from_bytes(text.c_str()) << std::endl;
			//result =ss.str();
		});
	});

	/* Output:
	<!DOCTYPE html>
	*/
}



std::string execOnline( string plyFile, std::vector<std::string>& paths, std::vector<std::string>& labels) {
	
	std::string result = HTTPStreamingAsync(plyFile).get();
	char delim = ',';
	auto alllines = split(result, delim);
	paths.push_back(alllines[0]);
	labels.push_back(alllines[1]);/*
	for (const auto& allline : alllines)
	{
		auto line = split(allline, delim);
		paths.push_back(line[0]);
		labels.push_back(line[1]);

	}*/
	//cout << "returnning.." << endl;
	return result;

}



std::string exec(string path2classifier, string path2plyFile, string flag, std::vector<std::string>& paths, std::vector<std::string>& labels) {
	string cmd = "cd " + path2classifier + " & python " + path2classifier + "requestClassification.py " + flag + " " + path2plyFile;
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != NULL)
			result += buffer.data();
	}

	auto delim = '\n';
	auto alllines = split(result, delim);
	delim = ',';
	for (const auto& allline : alllines)
	{
		auto line = split(allline, delim);
		paths.push_back(line[0]);
		labels.push_back(line[1]);

	}
	return result;

}

// Convert to colored depth image
cv::Mat convColoredDepth(cv::Mat& depthImg, float min_thresh = 0, float maxThresh = 0) {
	auto coloredDepth = depthImg.clone();

	double min;
	double max;
	if (min_thresh == 0 && maxThresh == 0) {
		cv::minMaxIdx(depthImg, &min, &max);
	}
	else {
		min = min_thresh;
		max = maxThresh;
	}
	coloredDepth -= min;
	cv::convertScaleAbs(coloredDepth, coloredDepth, 255 / (max - min));
	cv::applyColorMap(coloredDepth, coloredDepth, cv::COLORMAP_JET);

	return coloredDepth;
}
class StringBuilder {
private:
	std::string main;
	std::string scratch;

	const std::string::size_type ScratchSize = 1024;  // or some other arbitrary number

public:
	StringBuilder & append(const std::string & str) {
		scratch.append(str);
		if (scratch.size() > ScratchSize) {
			main.append(scratch);
			scratch.resize(0);
		}
		return *this;
	}

	const std::string & str() {
		if (scratch.size() > 0) {
			main.append(scratch);
			scratch.resize(0);
		}
		return main;
	}
};


string getPLYDataString( pcl::PointCloud<pcl::PointXYZRGBA>::Ptr points, pcl::PointCloud<pcl::Normal>::Ptr normals)
{
	StringBuilder sb;
	

	int point_num = points->size();
	sb.append("ply").append("\n");
	sb.append("format ascii 1.0").append("\n");
	sb.append("element vertex ").append(to_string(point_num)).append("\n");
	sb.append("property float x").append("\n");
	sb.append("property float y").append("\n");
	sb.append("property float z").append("\n");

	sb.append("property float normal_x").append("\n");
	sb.append("property float normal_y").append("\n");
	sb.append("property float normal_z").append("\n");

	sb.append("property uchar red").append("\n");
	sb.append("property uchar green").append("\n");
	sb.append("property uchar blue").append("\n");
	sb.append("property uchar alpha").append("\n");
	sb.append("end_header").append("\n");



	for (int i = 0; i < point_num; i++) {

		sb.append(to_string(points->at(i).x)).append(" ").append(to_string(points->at(i).y).append(" ").append(to_string(points->at(i).z)));

		if (normals != nullptr)
			sb.append(" ").append(to_string(normals->at(i).normal_x)).append(" ").append(to_string(normals->at(i).normal_y).append(" ").append(to_string(normals->at(i).normal_z)));

		else
			sb.append(" 0 0 0");

		sb.append(" ").append(to_string(static_cast<int>(points->at(i).r))).append(" ").append(to_string(static_cast<int>(points->at(i).g))).append(" ").append(to_string(static_cast<int>(points->at(i).b))).append(" 255\n");

	}


	return sb.str();
}


bool savePointCloudsPLY(string filename, pcl::PointCloud<pcl::PointXYZRGBA>::Ptr points, pcl::PointCloud<pcl::Normal>::Ptr normals)
{
	std::ofstream fout;
	fout.open(filename.c_str());
	if (fout.fail()) {
		cerr << "file open error:" << filename << endl;
		return false;
	}

	int point_num = points->size();

	fout << "ply" << endl;
	fout << "format ascii 1.0" << endl;
	fout << "element vertex " << point_num << endl;
	fout << "property float x" << endl;
	fout << "property float y" << endl;
	fout << "property float z" << endl;
	//if (normals != NULL)
	//{
	fout << "property float normal_x" << endl;
	fout << "property float normal_y" << endl;
	fout << "property float normal_z" << endl;
	//}

	fout << "property uchar red" << endl;
	fout << "property uchar green" << endl;
	fout << "property uchar blue" << endl;
	fout << "property uchar alpha" << endl;
	fout << "end_header" << endl;

	for (int i = 0; i < point_num; i++) {



		fout << points->at(i).x << " " << points->at(i).y << " " << points->at(i).z;
		if (normals != nullptr)
			fout << " " << normals->at(i).normal_x << " " << normals->at(i).normal_y << " " << normals->at(i).normal_z;
		else
			fout << " " << 0 << " " << 0 << " " << 0;

		fout << " " << static_cast<int>(points->at(i).r) << " " << static_cast<int>(points->at(i).g) << " " << static_cast<int>(points->at(i).b) << " " << 255 << endl;
	}

	fout.close();

	return true;
}


string to_binary(int n)
{
	string r;
	while (n != 0) { r = (n % 2 == 0 ? "0" : "1") + r; n /= 2; }
	return r;
}




pcl::PointCloud<pcl::PointXYZRGBA>::Ptr handleDetectedCluster(std::vector<pcl::PointIndices>::const_iterator it, pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_filtered, int j, pcl::PointCloud<pcl::Normal>::Ptr cloud_filtered_normal) {
	//HTTPStreamingAsync("lalala");
	clock_t t_start = clock();

	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZRGBA>);
	for (std::_Vector_const_iterator<std::_Vector_val<std::_Simple_types<int>>>::value_type indice : it->indices)
		cloud_cluster->points.push_back(cloud_filtered->points[indice]); //*
	cloud_cluster->width = cloud_cluster->points.size();
	cloud_cluster->height = 1;
	cloud_cluster->is_dense = true;



	if (debug) cout << "PointCloud representing the Cluster: " << cloud_cluster->points.size() << " data points." << std::endl;
	std::stringstream ss;
	ss << "cloud_cluster_" << j << ".pcd";


	//densifying the clouds?

	/*if (cloud_cluster->size() < 512)
	{
		return NULL;
		//pcl::MovingLeastSquares<pcl::PointXYZRGBA, pcl::PointXYZRGBA> mls;
		//mls.setInputCloud(cloud_cluster);
		//mls.setSearchRadius(0.03);
		//mls.setPolynomialFit(true);
		//mls.setPolynomialOrder(2);
		//mls.setUpsamplingMethod(pcl::MovingLeastSquares<pcl::PointXYZRGBA, pcl::PointXYZRGBA>::SAMPLE_LOCAL_PLANE);
		//mls.setUpsamplingRadius(0.01);// has 2 be larger than setUpsamplingStepSize
		//mls.setUpsamplingStepSize(0.008);//smaller increases the generated points
		//pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_smoothed(new pcl::PointCloud<pcl::PointXYZRGBA>());
		//mls.process(*cloud_cluster);
		//if (debug) cout << "upsampled cloud" << cloud_cluster->size() << endl;
	}*/


	auto ply_path = locationForOutputClouds + "cloud_cluster_" + to_string(j) + ".ply";

	pcl::PointCloud<pcl::Normal>::Ptr cluster_normal(new pcl::PointCloud<pcl::Normal>);
	pcl::NormalEstimation<pcl::PointXYZRGBA, pcl::Normal> ne;

	ne.setKSearch(100);



	ne.setInputCloud(cloud_cluster);
	ne.compute(*cluster_normal);



//	savePointCloudsPLY(ply_path, cloud_cluster, cluster_normal);
	string plyData = getPLYDataString(cloud_cluster, cluster_normal);
	std::vector<std::string> paths;
	std::vector<std::string> labels;


	string res = execOnline(plyData,  paths, labels);

	
	//string res = exec(path2classifier, ply_path, "--ply_path ", paths, labels);


	if (debug) cout << labels[0] << ":" << paths[0] << endl;
	if (::atof(paths[0].c_str()) < 0.5f)
	{
		return NULL;
	}



	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr currentModel(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr reducedcurrentModel(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::PointCloud<pcl::Normal>::Ptr currentModelNormals(new pcl::PointCloud<pcl::Normal>);

	string objID;

	int r = 0, g = 0, b = 0;
	if (labels[0] == "bird")
	{
		cout << "bird" << endl;

		r = 255;
		b = 255;
		objID = "01";
		currentModel = birdCloud;
		reducedcurrentModel = reducedbirdCloud;
		currentModelNormals = birdNormals;

	}
	else if (labels[0] == "house")
	{
		cout << "house" << endl;

		b = 255;
		objID = "05";

		currentModel = houseCloud;
		reducedcurrentModel = reducedhouseCloud;
		currentModelNormals = houseNormals;

	}
	else if (labels[0] == "cracker")
	{
		cout << "cracker" << endl;
		b = 255;
		g = 255;
		objID = "04";

		currentModel = crackerCloud;
		reducedcurrentModel = reducedcrackerCloud;
		currentModelNormals = crackerNormals;

	}
	else if (labels[0] == "can")
	{
		cout << "can" << endl;

		g = 255;
		objID = "03";

		currentModel = canCloud;
		reducedcurrentModel = reducedcanCloud;
		currentModelNormals = canNormals;


	}
	else if (labels[0] == "shoe")
	{
		cout << "shoe" << endl;

		r = 255;
		objID = "06";

		currentModel = shoeCloud;
		reducedcurrentModel = reducedshoeCloud;
		currentModelNormals = shoeNormals;


	}
	else
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		
		
		SetConsoleTextAttribute(hConsole, ++consoleColr);
		cout << labels[0] << endl;
		string binclr = to_binary(consoleColr);
		while (binclr.size()<3) {
			binclr = "0" + binclr;

		}
		r = int(binclr[0])*128;
		g = int(binclr[1]) * 128;//0 + (std::rand() % (255 - 0 + 1));
		b = int(binclr[2]) * 128;// 0 + (std::rand() % (255 - 0 + 1));



	}

	if (doAlignment)
	{




		if (debug) cout << "alignment" << endl;






		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr alignedModel(new pcl::PointCloud<pcl::PointXYZRGBA>);
		// Objects for storing the SHOT descriptors for the scene and model.
		pcl::PointCloud<pcl::SHOT352>::Ptr sceneDescriptors(new pcl::PointCloud<pcl::SHOT352>());
		pcl::PointCloud<pcl::SHOT352>::Ptr modelDescriptors(new pcl::PointCloud<pcl::SHOT352>());




		// Note: here you would compute or load the descriptors for both


		pcl::SHOTEstimationOMP<pcl::PointXYZRGBA, pcl::Normal, pcl::SHOT352> describer;
		describer.setRadiusSearch(0.05);
		//pcl::PointCloud<pcl::PointXYZRGBA>::Ptr sceneSampledCloudPtr(&sceneSampledCloud);

		describer.setInputCloud(cloud_cluster);
		describer.setInputNormals(cluster_normal);
		describer.setSearchSurface(cloud_cluster);
		describer.compute(*sceneDescriptors);


		describer.setInputCloud(reducedcurrentModel);
		describer.setInputNormals(currentModelNormals);
		describer.setSearchSurface(currentModel);
		describer.compute(*modelDescriptors);


		// Object for pose estimation.
		pcl::SampleConsensusPrerejective<pcl::PointXYZRGBA, pcl::PointXYZRGBA, pcl::SHOT352> pose;
		pose.setInputSource(reducedcurrentModel);
		pose.setInputTarget(cloud_cluster);
		pose.setSourceFeatures(modelDescriptors);
		pose.setTargetFeatures(sceneDescriptors);
		// Instead of matching a descriptor with its nearest neighbor, choose randomly between
		// the N closest ones, making it more robust to outliers, but increasing time.
		pose.setCorrespondenceRandomness(2);
		// Set the fraction (0-1) of inlier points required for accepting a transformation.
		// At least this number of points will need to be aligned to accept a pose.
		pose.setInlierFraction(0.25f);
		// Set the number of samples to use during each iteration (minimum for 6 DoF is 3).
		pose.setNumberOfSamples(3);
		// Set the similarity threshold (0-1) between edge lengths of the polygons. The
		// closer to 1, the more strict the rejector will be, probably discarding acceptable poses.
		pose.setSimilarityThreshold(0.6f);
		// Set the maximum distance threshold between two correspondent points in source and target.
		// If the distance is larger, the points will be ignored in the alignment process.
		pose.setMaxCorrespondenceDistance(0.01f);

		pose.align(*alignedModel);

		if (pose.hasConverged())
		{
			Eigen::Matrix4f transformation = pose.getFinalTransformation();
			Eigen::Matrix3f rotation = transformation.block<3, 3>(0, 0);
			Eigen::Vector3f translation = transformation.block<3, 1>(0, 3);

			std::cout << "Transformation matrix:" << std::endl << std::endl;
			printf("\t\t    | %6.3f %6.3f %6.3f | \n", rotation(0, 0), rotation(0, 1), rotation(0, 2));
			printf("\t\tR = | %6.3f %6.3f %6.3f | \n", rotation(1, 0), rotation(1, 1), rotation(1, 2));
			printf("\t\t    | %6.3f %6.3f %6.3f | \n", rotation(2, 0), rotation(2, 1), rotation(2, 2));
			std::cout << std::endl;
			printf("\t\tt = < %0.3f, %0.3f, %0.3f >\n", translation(0), translation(1), translation(2));


			// output transformation matrix 
			double score = pose.getFitnessScore();


			string rotationValues = to_string(rotation(0, 0))
				+ ", " + to_string(rotation(0, 1))
				+ ", " + to_string(rotation(0, 2))
				+ ", " + to_string(rotation(1, 0))
				+ ", " + to_string(rotation(1, 1))
				+ ", " + to_string(rotation(1, 2))
				+ ", " + to_string(rotation(2, 0))
				+ ", " + to_string(rotation(2, 1))
				+ ", " + to_string(rotation(2, 2));

			string translationValues = to_string(translation(0))
				+ ", " + to_string(translation(1))
				+ ", " + to_string(translation(2));


			// out writes to file XXXX_YY.txt
			double timeTaken = (double)(((double)(clock() - t_start)) / CLOCKS_PER_SEC);
			string path2output = teamOutput + "/" + challenge_name;
			if (boost::filesystem::create_directory(path2output))
			{
				std::cerr << "path2output created: " << path2output << std::endl;
			}

			string outputFileName = path2output + "/" + color_scene_filename + "_" + objID + "__" + to_string(j) + ".yml";

			if (debug) {
				cout << "outputFileName" << outputFileName << endl;
				cout << "colorSceneFilename" << color_scene_filename << endl;
				cout << "path2output" << path2output << endl;

			}
			boost::replace_all(outputFileName, "rgb\\", "");
			if (debug) {
				cout << "outputFileName" << outputFileName << endl;
			}
			boost::iostreams::stream_buffer<boost::iostreams::file_sink> buf(outputFileName);
			std::ostream osout(&buf);
			osout << "run_time: " + to_string(timeTaken) + "\r\n";
			osout << "ests:\r\n";
			osout << "- {score: " + to_string(score) + ", R : [" + rotationValues + "], t: [" + translationValues + "]}";



		}
		else std::cout << "Did not converge." << std::endl;





	}









	for (size_t n = 0; n < cloud_cluster->size(); n++)
	{
		//finalClouds[i]->points[l];
		cloud_cluster->points[n].r = r;
		//cloud_cluster->points[n].g = 255;
		cloud_cluster->points[n].b = b;
		cloud_cluster->points[n].g = g;

		//	if(debug) cout << "changed colors";
	}

	return cloud_cluster;



}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr  processCloud(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud) {



	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_f(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::UniformSampling<pcl::PointXYZRGBA> uniform_sampling;
	uniform_sampling.setInputCloud(cloud);
	uniform_sampling.setRadiusSearch(0.0006);
	uniform_sampling.filter(*cloud_filtered);


	if (debug) cout << "PointCloud after filtering has: " << cloud_filtered->points.size() << " data points." << std::endl; //*

	// Create the segmentation object for the planar model and set all the parameters
	pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
	pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
	pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_plane(new pcl::PointCloud<pcl::PointXYZRGBA>());
	pcl::PCDWriter writer;
	seg.setOptimizeCoefficients(true);
	seg.setModelType(pcl::SACMODEL_PLANE);
	seg.setMethodType(pcl::SAC_RANSAC);
	seg.setMaxIterations(100);
	seg.setDistanceThreshold(0.02);

	int i = 0, nr_points = (int)cloud_filtered->points.size();
	int prevSize = 0;
	int repeatCounter = 5;
	while (cloud_filtered->points.size() > 0.3 * nr_points)
	{
		// Segment the largest planar component from the remaining cloud
		seg.setInputCloud(cloud_filtered);
		seg.segment(*inliers, *coefficients);
		if (inliers->indices.size() == 0)
		{
			if (debug) cout << "Could not estimate a planar model for the given dataset." << std::endl;
			break;
		}

		// Extract the planar inliers from the input cloud
		pcl::ExtractIndices<pcl::PointXYZRGBA> extract;
		extract.setInputCloud(cloud_filtered);
		extract.setIndices(inliers);
		extract.setNegative(false);

		// Get the points associated with the planar surface
		extract.filter(*cloud_plane);
		if (debug) cout << "PointCloud representing the planar component: " << cloud_plane->points.size() << " data points." << std::endl;
		if (prevSize == cloud_plane->points.size())
		{
			repeatCounter--;
		}
		else
		{
			prevSize = cloud_plane->points.size();

		}
		// Remove the planar inliers, extract the rest
		if (repeatCounter != 0)
		{


			double z_min = -1.0f, z_max = 0;

			pcl::PointCloud<pcl::PointXYZRGBA>::Ptr hull_points(new pcl::PointCloud<pcl::PointXYZRGBA>());
			pcl::ConvexHull<pcl::PointXYZRGBA> hull;
			// hull.setDimension (2); // not necessarily needed, but we need to check the dimensionality of the output
			hull.setInputCloud(cloud_plane);
			hull.reconstruct(*hull_points);

		}


		extract.setIndices(inliers);

		extract.setNegative(true);
		extract.filter(*cloud_f);
		*cloud_filtered = *cloud_f;
		if (repeatCounter == 0)
		{
			repeatCounter = 5;
		}
	}

	// Creating the KdTree object for the search method of the extraction
	pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGBA>);
	tree->setInputCloud(cloud_filtered);

	std::vector<pcl::PointIndices> cluster_indices;
	pcl::EuclideanClusterExtraction<pcl::PointXYZRGBA> ec;
	//ec.setInputCloud(cloud_filtered);

	ec.setClusterTolerance(0.02); // 2cm
	ec.setMinClusterSize(100);
	ec.setMaxClusterSize(25000);

	ec.setSearchMethod(tree);
	ec.setInputCloud(cloud_filtered);
	ec.extract(cluster_indices);
	if (debug) cout << "Euclidean cluster done!" << endl;

	int j = 0;
	vector<	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr> finalClouds;// (new pcl::PointCloud<pcl::PointXYZRGBA>);

	pcl::PointCloud<pcl::Normal>::Ptr cloud_filtered_normal(new pcl::PointCloud<pcl::Normal>);
	pcl::NormalEstimation<pcl::PointXYZRGBA, pcl::Normal> ne;

	ne.setKSearch(100);



	ne.setInputCloud(cloud_filtered);
	ne.compute(*cloud_filtered_normal);


	consoleColr = 0;
	if (system("CLS")) system("clear");
	for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin(); it != cluster_indices.end(); ++it)
	{

		//THREADS should be created here!

		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_cluster = handleDetectedCluster(it, cloud_filtered, j, cloud_filtered_normal);

		if (cloud_cluster != NULL)
		{
			finalClouds.push_back(cloud_cluster);



			for (size_t b = 0; b < cloud_cluster->size(); b++)
			{
				cloud->push_back(cloud_cluster->points[b]);
			}
		}

		j++;
	}



	return cloud;

}
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr changeAlittle(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud) {
	for (size_t i = 0; i < cloud->size(); i++)
	{
		cloud->points[i].r = 0;
		cloud->points[i].g = 0;
	}
	return cloud;
}
class SimpleOpenNIViewer
{
public:
	SimpleOpenNIViewer() : viewer("PCL OpenNI Viewer") {}
	//			pcl::PointCloud<pcl::PointXYZRGBA>::Ptr modelCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);

	void cloud_cb_(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloud)
	{
		if (!viewer.wasStopped())
		{

			pcl::PointCloud<pcl::PointXYZRGBA>::Ptr modelCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
			if (debug) cout << "new cloud" << endl;

			std::vector<int> indices;
			pcl::removeNaNFromPointCloud(*cloud, *modelCloud, indices);
			viewer.showCloud(processCloud(modelCloud));




		}
	}

	void run()
	{
		pcl::Grabber* interface = new pcl::io::OpenNI2Grabber();

		boost::function<void(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr&)> f =
			boost::bind(&SimpleOpenNIViewer::cloud_cb_, this, _1);



		interface->registerCallback(f);

		interface->start();

		while (!viewer.wasStopped())
		{
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}

		interface->stop();
	}

	pcl::visualization::CloudViewer viewer;
};

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr generateSceneCloudsFromRGBD(string path) {



	// Loading depth image and color image


	boost::replace_all(path, "\\", "/");
	string colorFilename = path;

	//cout << path << endl;
	boost::replace_all(path, "rgb", "depth");
	string depthFilename = path;
	//cout << path << endl;

	cv::Mat depthImg = cv::imread(depthFilename, CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat colorImg = cv::imread(colorFilename, CV_LOAD_IMAGE_COLOR);
	cv::cvtColor(colorImg, colorImg, CV_BGR2RGB); //this will put colors right

	// Setting camera intrinsic parameters of depth camera
	float focalx = 539.8100;  // focal length
	float focaly = 539.8300;
	float px = 318.2700; // principal point x
	float py = 239.5600; // principal point y


	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr modelCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);


	//Create point clouds from depth image and color image using camera intrinsic parameters
	// (1) Compute 3D point from depth values and pixel locations on depth image using camera intrinsic parameters.
	for (int j = 0; j < depthImg.cols; j += 1)
	{
		for (int i = 0; i < depthImg.rows; i += 1)
		{
			auto point = Eigen::Vector4f((j - px)*depthImg.at<ushort>(i, j) / focalx, (i - py)*depthImg.at<ushort>(i, j) / focaly, depthImg.at<ushort>(i, j), 1);

			// (2) Translate 3D point in local coordinate system to 3D point in global coordinate system using camera pose.
			//point = poseMat *point;
			// (3) Add the 3D point to vertices in point clouds data.
			pcl::PointXYZRGBA p;
			p.x = point[0] / 1000.0f;
			p.y = point[1] / 1000.0f;
			p.z = point[2] / 1000.0f;
			p.r = colorImg.at<cv::Vec3b>(i, j)[0];
			p.g = colorImg.at<cv::Vec3b>(i, j)[1];
			p.b = colorImg.at<cv::Vec3b>(i, j)[2];
			p.a = 255;
			if (p.x == 0 && p.y == 0 && p.r == 0 && p.g == 0 && p.b == 0)
			{
				continue;
			}
			modelCloud->push_back(p);
			//	centroid.add(p);
		}
	}
	return modelCloud;
	//savePointCloudsPLY(outputCloudsDir + "\\" + modelName + "-" + to_string(modelIndex) + ".ply", modelCloud, NULL);

}




int main(int argc, char** argv)
{



	if (debug) cout << projectSrcDir << endl;
	if (doAlignment)
	{
		boost::filesystem::path dirTeamName(teamOutput.c_str());

		if (boost::filesystem::create_directory(dirTeamName))
		{
			std::cerr << "Teamname directory created: " << dirTeamName << std::endl;
		}

		if (pcl::io::loadPLYFile<pcl::PointXYZRGBA>(mainModelsPath + "/bird.ply", *birdCloud) == -1) { PCL_ERROR("Couldn't read file birdCloud.ply \n"); return (-1); }
		std::cout << "Loaded" << birdCloud->width * birdCloud->height << "points" << std::endl;
		if (pcl::io::loadPLYFile<pcl::PointXYZRGBA>(mainModelsPath + "/can.ply", *canCloud) == -1) { PCL_ERROR("Couldn't read file can.ply \n"); return (-1); }
		std::cout << "Loaded" << canCloud->width * canCloud->height << "points" << std::endl;
		if (pcl::io::loadPLYFile<pcl::PointXYZRGBA>(mainModelsPath + "/shoe.ply", *shoeCloud) == -1) { PCL_ERROR("Couldn't read file shoe.ply \n"); return (-1); }
		std::cout << "Loaded" << shoeCloud->width * shoeCloud->height << "points" << std::endl;
		if (pcl::io::loadPLYFile<pcl::PointXYZRGBA>(mainModelsPath + "/house.ply", *houseCloud) == -1) { PCL_ERROR("Couldn't read file house.ply \n"); return (-1); }
		std::cout << "Loaded" << houseCloud->width * houseCloud->height << "points" << std::endl;
		if (pcl::io::loadPLYFile<pcl::PointXYZRGBA>(mainModelsPath + "/cracker.ply", *crackerCloud) == -1) { PCL_ERROR("Couldn't read file crackerCloud.ply \n"); return (-1); }
		std::cout << "Loaded" << crackerCloud->width * crackerCloud->height << "points" << std::endl;

		pcl::console::print_highlight("Downsampling...\n");
		pcl::VoxelGrid<pcl::PointXYZRGBA> grid;
		const float leaf = 0.005f;
		grid.setLeafSize(leaf, leaf, leaf);


		grid.setInputCloud(birdCloud);
		grid.filter(*reducedbirdCloud);



		grid.setInputCloud(houseCloud);
		grid.filter(*reducedhouseCloud);


		grid.setInputCloud(canCloud);
		grid.filter(*reducedcanCloud);


		grid.setInputCloud(shoeCloud);
		grid.filter(*reducedshoeCloud);



		grid.setInputCloud(crackerCloud);
		grid.filter(*reducedcrackerCloud);




		pcl::NormalEstimation<pcl::PointXYZRGBA, pcl::Normal> ne;

		ne.setKSearch(100);



		ne.setInputCloud(birdCloud);
		ne.compute(*birdNormals);


		ne.setInputCloud(shoeCloud);
		ne.compute(*shoeNormals);

		ne.setInputCloud(canCloud);
		ne.compute(*canNormals);

		ne.setInputCloud(crackerCloud);
		ne.compute(*crackerNormals);

		ne.setInputCloud(houseCloud);
		ne.compute(*houseNormals);


	}



	if (live)
	{


		SimpleOpenNIViewer v;
		v.run();
	}
	else {


		for (auto modelsIT : directory_iterator(scenesRGBDMainPath))
		{

			string modelPathIT = modelsIT.path().string();//path to a model folder, e.g. bird
			boost::replace_all(modelPathIT, "\\", "/");
			dir_challenge = modelPathIT;

			string modelName = modelPathIT.substr(modelPathIT.find_last_of("/") + 1);
			string modelRGBDir = modelPathIT + "/rgb/";
			string modelDepthDir = modelPathIT + "/depth/";
			challenge_name = modelName;
			if (debug) {
				cout << "###################################################################" << endl;
				cout << "scenesRGBDMainPath" << scenesRGBDMainPath << endl; //points to /test
				cout << "challengeName" << challenge_name << endl;//01 , 02 ...etc
				cout << "dirChallenge" << dir_challenge << endl;
				cout << "###################################################################" << endl;

			}




			//int i = 0;
			//int modelIndex = -1;
			for (boost::filesystem::directory_entry it : directory_iterator(modelRGBDir))
			{

				color_scene_filename = it.path().string().substr(it.path().string().find_last_of("/") + 1);
				boost::replace_all(color_scene_filename, ".png", "");

				//modelIndex++;
				pcl::PointCloud<pcl::PointXYZRGBA>::Ptr scene = processCloud(generateSceneCloudsFromRGBD(it.path().string()));
				pcl::visualization::PCLVisualizer viewer3("scene instances");
				viewer3.addPointCloud(scene, "scene");
				while (!viewer3.wasStopped())
				{
					viewer3.spinOnce(100);
				}


			}
		}

	}

	return (0);
}

