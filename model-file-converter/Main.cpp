#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

// This represents the output type of this program (i.e., the input to the DirectX vertex buf)
struct DXVertexInput {
	float posX, posY, posZ;
	float texU, texV;
	float normX, normY, normZ;

	DXVertexInput() {
		posX = posY = posZ = texU = texV = normX = normY = normZ = 0.0f;
	}

	DXVertexInput(
		const std::vector<float>& pos,
		const std::vector<float>& tex,
		const std::vector<float>& norm)
	{
		posX = pos.at(0); posY = pos.at(1); posZ = pos.at(2);
		texU = tex.at(0); texV = 1.0 - tex.at(1);
		normX = norm.at(0); normY = norm.at(1); normZ = norm.at(2);
	}
};

int loadLineVectors(
	std::string filename,
	std::vector<std::string>& vLines,
	std::vector<std::string>& vtLines,
	std::vector<std::string>& vnLines,
	std::vector<std::string>& fLines)
{
	vLines.clear();
	vtLines.clear();
	vnLines.clear();
	fLines.clear();

	std::ifstream fin;
	fin.open(filename);

	std::string line;
	int lineCount = 0;
	while (std::getline(fin, line)) {
		if (line.rfind("v ", 0) == 0) {
			vLines.push_back(line);
		}
		else if (line.rfind("vt ", 0) == 0) {
			vtLines.push_back(line);
		}
		else if (line.rfind("vn ", 0) == 0) {
			vnLines.push_back(line);
		}
		else if (line.rfind("f ", 0) == 0) {
			fLines.push_back(line);
		}
		else {
			printf("Skipping line with unknown prefix: '%s'\n", line.c_str());
			lineCount--;
		}
		lineCount++;
	}
	return lineCount;
}

int parseInputLines(
	const std::vector<std::string> &in, std::vector<std::vector<float>> &out, int coords) 
{
	for (int i = 0; i < in.size(); i++) {
		out.at(i) = std::vector<float>(coords);

		std::istringstream iss(in.at(i));
		std::string coordToken;
		if (!(iss >> coordToken)) {
			printf("ERROR: No tokens in line %d: '%s'. Empty?\n", i, in.at(i).c_str());
			return -1;
		}
		int tokensParsed;
		for (tokensParsed = 0; tokensParsed < coords; tokensParsed++) {
			if (!(iss >> coordToken)) {
				printf("ERROR: Not enough tokens in input line: '%s'\n", in.at(i).c_str());
				return -1;
			}
			try {
				out.at(i).at(tokensParsed) = std::stof(coordToken);
			}
			catch (...) {
				printf("ERROR: Failed to convert token '%s' in line '%s' to float.\n",
					coordToken.c_str(), in.at(i).c_str());
				return -1;
			}
		}
		// Check if any non-whitespace chars left in this line
		while (!iss.eof()) {
			std::string firstExtra;
			iss >> firstExtra;
			if (firstExtra.empty()) continue;
			printf("ERROR: Too many tokens in line '%s'. Expected %d. First extra='%s'\n", 
				in.at(i).c_str(), coords, firstExtra.c_str());
			return -1;
		}
	}
}

int parseFaceInputLines(
	std::vector<std::string> &in, std::vector<std::vector<std::string>> &out, int coords) 
{
	for (int i = 0; i < in.size(); i++) {
		out.at(i) = std::vector<std::string>(coords);

		std::istringstream line(in.at(i));
		std::string coordToken;
		if (!(line >> coordToken)) {
			printf("ERROR: No tokens in line %d: '%s'. Empty?\n", i, in.at(i).c_str());
			return -1;
		}
		int tokensParsed;
		for (tokensParsed = 0; tokensParsed < coords; tokensParsed++) {
			if (!(line >> coordToken)) {
				printf("ERROR: Not enough tokens in input line: '%s'\n", in.at(i).c_str());
				return -1;
			}
			out.at(i).at(tokensParsed) = coordToken;
		}
		while (!line.eof()) {
			std::string firstExtra;
			line >> firstExtra;
			if (firstExtra.empty()) continue;
			printf("ERROR: Too many tokens in line '%s'. Expected %d. First extra='%s'\n",
				in.at(i).c_str(), coords, firstExtra.c_str());
			return -1;
		}
	}
}

std::vector<DXVertexInput> materializeFaces(
	std::vector<std::vector<float>> v,
	std::vector<std::vector<float>> vt,
	std::vector<std::vector<float>> vn,
	std::vector<std::vector<std::string>> faces)
{
	std::vector<DXVertexInput> dxInputs(faces.size() * 3);

	for (int i = 0; i < faces.size(); i++) {
		std::vector<std::string> face = faces.at(i);
		for (int j = 0; j < 3; j++) {
			std::string faceVertex = face.at(j);
			int off1 = faceVertex.find("/");
			int off2 = faceVertex.find("/", off1 + 1);
			
			// Woof.
			int vnIndex = stoi(faceVertex.substr(off2 + 1)) - 1;
			int vtIndex = stoi(faceVertex.substr(off1 + 1, off2 - (off1 + 1))) - 1;
			int vIndex = stoi(faceVertex.substr(0, off1)) - 1;

			const std::vector<float> vertex = v.at(vIndex);
			const std::vector<float> texel = vt.at(vtIndex);
			const std::vector<float> normal = vn.at(vnIndex);

			dxInputs.at(i * 3 + j) = DXVertexInput(vertex, texel, normal);
		}
	}

	return dxInputs;
}



int main(int argc, char* argv[]) {
	printf("Model converter started.  argc=%d\n", argc);
	if (argc < 3) {
		printf("ERROR: missing input or output filename parameter\n");
		return -5;
	}

	std::string inputModelFilename = argv[1];
	std::string outputModelFilename = argv[2];
	printf("Attempting to convert model file: %s\n", inputModelFilename.c_str());

	std::vector<std::string> vLines, vtLines, vnLines, fLines;
	int lineCount = loadLineVectors(inputModelFilename, vLines, vtLines, vnLines, fLines);
	printf("Extracted %d relevant lines from model file.\n", lineCount);

	//printf("Vertices:\n");
	//for (int i = 0; i < vLines.size(); i++) {
	//	std::cout << vLines[i] << std::endl;
	//}
	//printf("Texels:\n");
	//for (int i = 0; i < vtLines.size(); i++) {
	//	std::cout << vtLines[i] << std::endl;
	//}
	//printf("Normals:\n");
	//for (int i = 0; i < vnLines.size(); i++) {
	//	std::cout << vnLines[i] << std::endl;
	//}
	//printf("Faces:\n");
	//for (int i = 0; i < fLines.size(); i++) {
	//	std::cout << fLines[i] << std::endl;
	//}

	std::vector<std::vector<float>> v(vLines.size());
	std::vector<std::vector<float>> vt(vtLines.size());
	std::vector<std::vector<float>> vn(vnLines.size());
	std::vector<std::vector<std::string>> f(fLines.size());

	if (parseInputLines(vLines, v, 3) < 0) {
		printf("ERROR: parseInputLines failed, aborting.\n");
		return -10;
	}
	//printf("Loaded vertices vector:\n");
	//for (int i = 0; i < v.size(); i++) {
	//	std::vector<float> vertex = v.at(i);
	//	for (int j = 0; j < vertex.size(); j++) {
	//		std::cout << vertex.at(j) << " ";
	//	}
	//	std::cout << std::endl;
	//}

	if (parseInputLines(vtLines, vt, 2) < 0) {
		printf("ERROR: parseInputLines failed, aborting.\n");
		return -10;
	}
	//printf("Loaded texels vector:\n");
	//for (int i = 0; i < vt.size(); i++) {
	//	std::vector<float> texel = vt.at(i);
	//	for (int j = 0; j < texel.size(); j++) {
	//		std::cout << texel.at(j) << " ";
	//	}
	//	std::cout << std::endl;
	//}

	if (parseInputLines(vnLines, vn, 3) < 0) {
		printf("ERROR: parseInputLines failed, aborting.\n");
		return -10;
	}
	//printf("Loaded normals vector:\n");
	//for (int i = 0; i < vn.size(); i++) {
	//	std::vector<float> normal = vn.at(i);
	//	for (int j = 0; j < normal.size(); j++) {
	//		std::cout << normal.at(j) << " ";
	//	}
	//	std::cout << std::endl;
	//}

	if (parseFaceInputLines(fLines, f, 3) < 0) {
		printf("ERROR:parseFaceInputLines failed, aborting.\n");
		return -15;
	}
	//printf("Loaded faces vector:\n");
	//for (int i = 0; i < f.size(); i++) {
	//	std::vector<std::string> face = f.at(i);
	//	for (int j = 0; j < face.size(); j++) {
	//		std::cout << face.at(j) << " ";
	//	}
	//	std::cout << std::endl;
	//}

	std::vector<DXVertexInput> dxInputs = materializeFaces(v, vt, vn, f);
	printf("Materialized!\n");
	//for (int i = 0; i < dxInputs.size(); i++) {
	//	DXVertexInput dxi = dxInputs.at(i);
	//	printf("%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
	//		dxi.posX, dxi.posY, dxi.posZ, dxi.texU, dxi.texV, dxi.normX, dxi.normY, dxi.normZ);
	//}

	std::ofstream outfile(outputModelFilename);
	char strBuf[100];
	sprintf(strBuf, "%d\n", (int)dxInputs.size());
	outfile << strBuf;
	for (int i = 0; i < dxInputs.size(); i++) {
		DXVertexInput dxi = dxInputs.at(i);
		sprintf(strBuf, "%f %f %f %f %f %f %f %f\n",
			dxi.posX, dxi.posY, dxi.posZ, dxi.texU, dxi.texV, dxi.normX, dxi.normY, dxi.normZ);
		outfile << strBuf;
	}
	outfile.close();

	printf("Output written to: %s\n", outputModelFilename.c_str());
	return 0;
}