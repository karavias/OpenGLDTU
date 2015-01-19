#ifndef __SIMPLE_RESOURCE_LOADER
#define __SIMPLE_RESOURCE_LOADER

#include <map>
#include <string>

class SimpleResourceLoader
{
public:
	SimpleResourceLoader(std::string resourcefilename);
	~SimpleResourceLoader(void);

	std::string Get(std::string key);
	int GetInt(std::string key);
private:
	std::map<std::string, std::string> map;

	void LoadAndParseFile(std::string resourcefilename);
};

unsigned char *load3DTexture(const char *name, int xsize, int ysize, int slices);

#endif // __SIMPLE_RESOURCE_LOADER
