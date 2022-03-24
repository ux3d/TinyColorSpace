#include <cstdio>
#include <string>
#include <vector>

#include <OpenImageIO/imageio.h>

#include "TinyColorSpace.h"

using namespace OIIO;

void generateMat3(const glm::mat3& m)
{
	printf("const mat3 m = mat3(\n");
	for (size_t r = 0; r < 3; r++)
	{
		std::string comment;

		printf("   ");
		for (size_t c = 0; c < 3; c++)
		{
			if (m[r][c] >= 0.0)
			{
				printf(" ");
			}
			printf("%f", m[r][c]);
			if (m[c][r] >= 0.0)
			{
				comment += " ";
			}
			comment += std::to_string(m[c][r]);
			if (c != 2 || r != 2)
			{
				printf(", ");
				comment += ", ";
			}
			else
			{
				printf("  ");
			}
		}
		printf("   // %s", comment.c_str());
		printf("\n");
	}
	printf(");\n\n");
}

//

class ImageData {

private:

	std::vector<float> pixels;
	uint32_t channels = 0;
	uint32_t width = 0;
	uint32_t height = 0;

public:

	ImageData(uint32_t channels, uint32_t width, uint32_t height)
	{
		if (channels < 1 || channels > 4)
		{
			return;
		}
		// Only support 8K today
		if (width < 1 || width > 7680)
		{
			return;
		}
		if (height < 1 || height > 4320)
		{
			return;
		}

		//

		this->pixels.resize(channels * width * height);
		this->channels = channels;
		this->width = width;
		this->height = height;
	}

	uint32_t getChannels() const
	{
		return channels;
	}

	uint32_t getWidth() const
	{
		return width;
	}

	uint32_t getHeight() const
	{
		return height;
	}

	const float* getPixelsData() const
	{
		return (const float*)pixels.data();
	}

	//

	glm::vec4 getColor(uint32_t x, uint32_t y)
	{
		glm::vec4 result = glm::vec4(0.0, 0.0, 0.0, 0.0);

		if (x >= width || y >= height)
		{
			return result;
		}

		for (uint32_t c = 0; c < channels; c++)
		{
			result[c] = pixels[channels * width * y + channels * x + c];
		}

		return result;
	}

	void setColor(uint32_t x, uint32_t y, const glm::vec4& color)
	{
		if (x >= width || y >= height)
		{
			return;
		}

		for (uint32_t c = 0; c < channels; c++)
		{
			pixels[channels * width * y + channels * x + c] = color[c];
		}
	}

	void fill(const glm::vec4& color)
	{
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t c = 0; c < channels; c++)
				{
					pixels[channels * width * y + channels * x + c] = color[c];
				}
			}
		}
	}

};

//

int main(int argc, char* argv[])
{
	glm::mat3 test = glm::mat3(1.0);

	//

	test = XYZ_2_AP1 * D65_2_D60 * SRGB_2_XYZ;
	generateMat3(test);

	generateMat3(RRT_SAT);

	test = RRT_SAT * test;
	generateMat3(test);

	printf("---\n\n");

	//

	test = XYZ_2_SRGB * D60_2_D65 * AP1_2_XYZ;
	generateMat3(test);

	generateMat3(ODT_SAT);

	test = test * ODT_SAT;
	generateMat3(test);

	//
	//
	//

	ImageData imageData(3, 512, 512);
	imageData.fill(glm::vec4(1.0, 0.0, 0.0, 0.0));

	//

	const char* filename = "test.exr";
	std::unique_ptr<ImageOutput> output = ImageOutput::create(filename);
	if (!output)
	{
		return -1;
	}
	ImageSpec spec (imageData.getWidth(), imageData.getHeight(), imageData.getChannels(), TypeDesc::FLOAT);
	output->open(filename, spec);
	output->write_image(TypeDesc::FLOAT, imageData.getPixelsData());
	output->close();

	printf("Saved '%s'\n", filename);

	return 0;
}
