#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <sstream>
#include <vector>
#include <map>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>


std::map<std::string, std::vector<double> > curveMins;
std::map<std::string, std::vector<double> > curveMaxs;
std::vector<double> maxs;
std::vector<double> mins;


void tokenize(const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of( delimiters, 0 );

	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of( delimiters, lastPos );

	while ( std::string::npos != pos || std::string::npos != lastPos )
	{
		// Found a token, add it to the vector.
		tokens.push_back( str.substr( lastPos, pos - lastPos ) );

		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of( delimiters, pos );

		// Find next "non-delimiter"
		pos = str.find_first_of( delimiters, lastPos );
	}
}

void process(std::vector<double>& curve, std::string name)
{
	if (curve.size() < 2)
		return;

	double min = curve[1];
	double max = curve[1];
	for (size_t i = 0; i < curve.size(); ++i)
	{
		if (i % 4 == 0)
		{
			curve[i] /= 1000;
		}
		else if (i % 4 == 1)
		{
			if (curve[i] <= min)
				min = curve[i];
			if (curve[i] >= max)
				max = curve[i];
		}
	}
	mins.push_back(min);
	maxs.push_back(max);
	curveMins[name].push_back(min);
	curveMaxs[name].push_back(max);

	/*
	// normalize to 0 to 1
	if (fabs(max - min) > 0.0000001)
	{
		for (size_t i = 0; i < curve.size(); ++i)
		{
			if (i % 4 == 1)
			{
				curve[i] = (curve[i] - min) / (max - min);
			}
		}
	}
	*/
}

void generateCurveBML(const std::string& iFileName, const std::string& oFileName)
{
	std::ifstream ifile(iFileName);
	std::ofstream ofile(oFileName);

	if (!ifile.good() || !ofile.good())
		return;

	std::vector<std::string> lines;
	while (!ifile.eof())
	{
		std::string line;
		std::getline(ifile, line);
		if (line != "")
			lines.push_back(line);
	}

	std::vector<std::string> curveNames;
	std::vector<double> defaultData;
	curveNames.push_back("jaw_rot");
	defaultData.push_back(0.210);
	curveNames.push_back("lower_lip_ftuck");
	defaultData.push_back(0);
	curveNames.push_back("upper_lip_raise");
	defaultData.push_back(-2.96);
	curveNames.push_back("cheek_hollow");
	defaultData.push_back(0);
	curveNames.push_back("lower_lip_roll");
	defaultData.push_back(0);
	curveNames.push_back("jaw_thrust");
	defaultData.push_back(0);
	curveNames.push_back("lip_corner_zip");
	defaultData.push_back(0);
	curveNames.push_back("lower_lip_raise");
	defaultData.push_back(0);
	curveNames.push_back("lip_rounding");
	defaultData.push_back(6.106);
	curveNames.push_back("lip_retraction");
	defaultData.push_back(2.809);
	curveNames.push_back("tongue_tip_y");
	defaultData.push_back(-0.06);
	curveNames.push_back("tongue_tip_z");
	defaultData.push_back(0.05);
	

	std::vector<std::vector<double> > data;
	data.resize(curveNames.size());
	for (size_t i = 0; i < data.size(); ++i)
		data[i].resize(lines.size() * 4);		// * 4 is to be appliance with facefx data

	for (size_t i = 0; i < lines.size(); ++i)
	{
		std::vector<std::string> tokens;
		tokenize(lines[i], tokens, "");
		if (tokens.size() != (curveNames.size() + 1))
		{
			printf("Bad input data! (%s)\n", iFileName.c_str());
			continue;
		}
		for (size_t j = 1; j < tokens.size(); ++j)
		{
			data[j - 1][i * 4 + 0] = atof(tokens[0].c_str());
			data[j - 1][i * 4 + 1] = atof(tokens[j].c_str());
			data[j - 1][i * 4 + 2] = 0;
			data[j - 1][i * 4 + 3] = 0;
		}
	}

	std::vector<std::string> pCurveNames;
	std::vector<std::vector<double> > pCurveData;
	for (size_t i = 0; i < data.size(); ++i)
	{
		process(data[i], curveNames[i]);
	}

	for (size_t i = 0; i < curveNames.size(); ++i)
	{
		std::vector<double> data1;
		std::vector<double> data2;
		for (size_t j = 0; j < data[i].size(); ++j)
		{
			if (j % 4 == 1)
			{
				if (data[i][j] < defaultData[i])
				{
					data1.push_back(data[i][j - 1]);
					data1.push_back(data[i][j]);
					data1.push_back(data[i][j + 1]);
					data1.push_back(data[i][j + 2]);
				}
				else
				{
					data2.push_back(data[i][j - 1]);
					data2.push_back(data[i][j]);
					data2.push_back(data[i][j + 1]);
					data2.push_back(data[i][j + 2]);
				}
			}
		}
		if (data1.size() > 0)
		{	
			std::string name = curveNames[i] + "_min";
			pCurveNames.push_back(name);
			pCurveData.push_back(data1);
		}
		if (data2.size() > 0)
		{
			std::string name = curveNames[i] + "_max";
			pCurveNames.push_back(name);
			pCurveData.push_back(data2);
		}
	}

	ofile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
	ofile << "<bml>" << std::endl;
	ofile << "	<speech id=\"sp1\" start=\"0.0\" ready=\"0.1\" stroke=\"0.1\" relax=\"0.2\" end=\"0.2\">" << std::endl;
	ofile << "		<text>" << std::endl;
	ofile << "		</text>" << std::endl;
	ofile << "		<description level=\"1\" type=\"audio/x-wav\">" << std::endl;
	ofile << "			<file ref=\"The-Break-Up-01-Aniston-ComeOn\" />" << std::endl;
	ofile << "		</description>" << std::endl;
	ofile << "	</speech>" << std::endl;
	ofile << "	<curves>" << std::endl;
	for (size_t i = 0; i < pCurveNames.size(); ++i)
	{
		ofile << "		<curve name=\"" << pCurveNames[i] << "\" num_keys=\"" << pCurveData[i].size() / 4 << "\" owner=\"massaro\">";
		for (size_t j = 0; j < pCurveData[i].size(); ++j)
			ofile << pCurveData[i][j] << " ";
		ofile << "</curve>" << std::endl;
	}
	ofile << "	</curves>" << std::endl;
	ofile << "</bml>" << std::endl;
	ofile.close();

}

double getMin(std::vector<double> d)
{
	double m = d[0];
	for (size_t i = 0; i < d.size(); ++i)
	{
		if (d[i] <= m)
			m = d[i];
	}
	return m;
}

double getMax(std::vector<double> d)
{
	double m = d[0];
	for (size_t i = 0; i < d.size(); ++i)
	{
		if (d[i] >= m)
			m = d[i];
	}
	return m;
}


void converterFromDomToBML(const std::string& rootDirectory)
{
	mins.clear();
	maxs.clear();

	boost::filesystem::path path(rootDirectory);
	if (!boost::filesystem::exists(path))
	{
		std::cout << "Directory " << rootDirectory << " does not exist, exiting..." << std::endl;
		return;
	}

	boost::filesystem::directory_iterator endIter;
	for (boost::filesystem::directory_iterator dirIter(path); 
		dirIter != endIter;
		dirIter++)
	{
		if (!boost::filesystem::is_directory(dirIter->status()))
		{
			std::string fileName = dirIter->path().string();
			std::string filebasename = boost::filesystem::basename(fileName);
			std::string fileextension = boost::filesystem::extension(fileName);
			if (fileextension == ".par")
			{
				std::string iFileName = rootDirectory + filebasename + ".par";
				std::string oFileName = rootDirectory + filebasename + ".bml";
				generateCurveBML(iFileName, oFileName);
			}
		}
	}

	double gMin = getMin(mins);
	double gMax = getMax(maxs);
	printf("Global min: %f; Global max: %f\n", gMin, gMax);

	std::map<std::string, std::vector<double> >::iterator iter = curveMins.begin();
	for (; iter != curveMins.end(); iter++)
	{
		double localMin = getMin(iter->second);
		printf("%s: %f\n", iter->first.c_str(), localMin);
	}

	iter = curveMaxs.begin();
	for (; iter != curveMaxs.end(); iter++)
	{
		double localMax = getMax(iter->second);
		printf("%s: %f\n", iter->first.c_str(), localMax);
	}
}

void generateDiphoneBML(const std::string& iFileName, const std::string& oFileName, const std::map<std::string, std::string> mapping)
{
	std::ifstream ifile(iFileName);
	std::ofstream ofile(oFileName);

	if (!ifile.good() || !ofile.good())
		return;

	std::vector<std::string> lines;
	while (!ifile.eof())
	{
		std::string line;
		std::getline(ifile, line);
		if (line != "")
			lines.push_back(line);
	}
	lines.erase(lines.begin(), lines.begin() + 2);

	std::vector<double> startTimes;
	std::vector<double> endTimes;
	std::vector<std::string> phonemes;
	for (size_t i = 0; i < lines.size(); ++i)
	{
		std::vector<std::string> tokens;
		tokenize(lines[i], tokens, " ");
		if (tokens.size() != 3)
		{
			std::cout << "Mal-formatted output from cslu: " << lines[i] << std::endl;
			continue;
		}
		std::map<std::string, std::string>::const_iterator iter = mapping.find(tokens[2]);
		if (iter == mapping.end())
		{
			std::cout << "Mal-formatted output from mapping: " << lines[i] << std::endl;
			continue;
		}
		startTimes.push_back(atof(tokens[0].c_str()) / 1000);
		endTimes.push_back(atof(tokens[1].c_str()) / 1000);
		phonemes.push_back(iter->second);
	}

	ofile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
	ofile << "<bml>" << std::endl;
	ofile << "	<speech id=\"sp1\" start=\"0.0\" ready=\"0.1\" stroke=\"0.1\" relax=\"0.2\" end=\"0.2\">" << std::endl;
	ofile << "		<text>" << std::endl;
	ofile << "		</text>" << std::endl;
	ofile << "		<description level=\"1\" type=\"audio/x-wav\">" << std::endl;
	ofile << "			<file ref=\"The-Break-Up-01-Aniston-ComeOn\" />" << std::endl;
	ofile << "		</description>" << std::endl;
	ofile << "	</speech>" << std::endl;
	for (size_t i = 0; i < phonemes.size(); ++i)
	{
		ofile << "	<lips viseme=\"" << phonemes[i] << "\" articulation=\"1.0\" start=\"" << startTimes[i] << "\" ready=\"" << startTimes[i] << "\" relax=\"" << endTimes[i] << "\" end=\"" << endTimes[i] << "\" />" << std::endl;
	}
	ofile << "	<curves>" << std::endl;
	ofile << "	</curves>" << std::endl;
	ofile << "</bml>" << std::endl;
	ofile.close();

}

void convertFromCSLUToBML(const std::string& rootDirectory, const std::string& mappingFileFullPath)
{
	// get mapping
	std::map<std::string, std::string> worldbet2diphone;
	std::ifstream mappingFile(mappingFileFullPath);
	std::string line;
	while (mappingFile.good())
	{
		getline(mappingFile, line);
		if (line == "")	continue;
		std::vector<std::string> tokens;
		tokenize(line, tokens, "	");
		if (tokens.size() != 4)
		{
			std::cout << "Mal-formatted mapping file" << std::endl;
			continue;
		}
		worldbet2diphone[tokens[0]] = tokens[2];
	}

	for (std::map<std::string, std::string>::iterator iter = worldbet2diphone.begin(); iter != worldbet2diphone.end(); ++iter)
	{
		std::cout << iter->first << "		" << iter->second << std::endl;
	}
	std::cout << "Number of mapping: " << worldbet2diphone.size() << std::endl;
	mappingFile.close();

	// parse files
	boost::filesystem::path path(rootDirectory);
	if (!boost::filesystem::exists(path))
	{
		std::cout << "Directory " << rootDirectory << " does not exist, exiting..." << std::endl;
		return;
	}

	boost::filesystem::directory_iterator endIter;
	for (boost::filesystem::directory_iterator dirIter(path); 
		dirIter != endIter;
		dirIter++)
	{
		if (!boost::filesystem::is_directory(dirIter->status()))
		{
			std::string fileName = dirIter->path().string();
			std::string filebasename = boost::filesystem::basename(fileName);
			std::string fileextension = boost::filesystem::extension(fileName);
			if (fileextension == ".txt1")
			{
				std::string iFileName = rootDirectory + "/" + filebasename + ".txt1";
				std::string oFileName = rootDirectory + "/" + filebasename + ".bml";
				generateDiphoneBML(iFileName, oFileName, worldbet2diphone);
			}
		}
	}
}

int main(int argc, char* argv[])
{
//	std::string rootDirectory = "E:/Projects/massaro/files/";
//	converterFromDomToBML(rootDirectory);
//	convertFromCSLUToBML("E:/Projects/cslu/files", "E:/Projects/cslu/mapping.txt");

	printf("Number of arguments: %d\n", argc);
	for (int i = 0; i < argc; ++i)
	{
		printf("%s\n", argv[i]);
	}
	if (argc != 3)
	{
		printf("Input parameters: <root directory for the files need processing> <location of mapping file from worldbet to SmartBody phoneme set>.\n");
		return 0;
	}
	convertFromCSLUToBML(argv[1], argv[2]);
	return 0;
}