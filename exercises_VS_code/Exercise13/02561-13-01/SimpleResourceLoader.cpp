#include "SimpleResourceLoader.h"

#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

unsigned char *load3DTexture(const char *name, int xsize, int ysize, int slices){
	using namespace std;
	ifstream file (name, ios::in|ios::binary);
	if(!file.is_open())
	{    
		cout << "Could not open file" << endl;
		exit(1);
	} else {
		unsigned char *texture = new unsigned char[xsize*ysize*slices];
		memset(texture, 0, xsize*ysize*slices);
		int sliceSize = xsize*ysize;
		for(int k=0;k<slices;++k)
			{
				file.read(&reinterpret_cast<char*>(texture)[sliceSize*k],sliceSize);

				if (file.fail())
					{
						cout << "Warning (probably harmless): Failed to read slice "
								 << k << endl;

						break;
					}
			}
		file.close();
		return texture;
	}	 
}

SimpleResourceLoader::SimpleResourceLoader(std::string resourcefilename) {
	LoadAndParseFile(resourcefilename);
}


SimpleResourceLoader::~SimpleResourceLoader(void) {
}

void SimpleResourceLoader::LoadAndParseFile(string resourcefilename) {
	ifstream file (resourcefilename, ios::in);
	if(!file.is_open()) {    
		cout << "Could not open file" << resourcefilename << endl;
		exit(1);
	} else {
		char buffer[500];
		while (!file.eof()){
			file.getline(buffer, 500);
			if (strchr (buffer, '=')!= NULL){ // if line contains =
				char * pch = strtok (buffer,"\t \"="); // split line to get key
				if (pch != NULL){
					string key(pch);
					pch = strtok (NULL, "\t \"="); // split line to get value
					if (pch != NULL){
						string value(pch);
						cout<<key<<"="<<value<<endl;
						map[key] = value;
					}
				}
			}
		}
		file.close();
	}
}

string SimpleResourceLoader::Get(string key) {
	return map[key];
}

int SimpleResourceLoader::GetInt(string key) {
	string value = map[key];
	return atoi(value.c_str());
}
