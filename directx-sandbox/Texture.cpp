#include "Texture.h"

Texture::Texture() {
	this->pTargaData = nullptr;		// Raw loaded .tga data
	this->pTexture = nullptr;		// Actual DirectX texture
	this->pTextureView = nullptr;	// ?? Mystery
}

Texture::Texture(const Texture& other) {
}

Texture::~Texture() {
}

bool Texture::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename) {
	int width, height;
	bool result = LoadTarga(filename, height, width);
	if (!result) {
		printf("ERROR: Failed to load Targa \"%s\"\n", filename);
		return false;
	}
	printf("Targa loaded from \"%s\"!\n", filename);
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA texture
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT hResult = device->CreateTexture2D(&textureDesc, nullptr, &this->pTexture);
	if (FAILED(hResult)) {
		printf("ERROR: Failed to create empty Texture2D.\n");
		return false;
	}

	unsigned int rowPitch = (width * 4) * sizeof(unsigned char);

	// This actually copies the targa data into the DirectX texture. This can also be done with Map
	// and Unmap (like we did with the world/view/projection matrices into the Model), but since 
	// this loading is not done every frame UpdateSubresource is preferred. Map/Unmap is quicker to do
	// (good for when it's done each frame), but UpdateSubresource gets your data cache retention 
	// preference. If DirectX expects data to be updated every frame, it's not gonna bother to cache it.
	// Tell DirectX this by using D3D11_USAGE_DEFAULT vs D3D11_USAGE_DYNAMIC
	deviceContext->UpdateSubresource(this->pTexture, 0, nullptr, this->pTargaData, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hResult = device->CreateShaderResourceView(this->pTexture, &srvDesc, &this->pTextureView);
	if (FAILED(hResult)) {
		printf("ERROR: Failed to create shader resource view.\n");
		return false;
	}

	deviceContext->GenerateMips(this->pTextureView);

	// Now that' we've loaded the targa data into the DirectX texture, we don't need it anymore
	delete[] this->pTargaData;
	this->pTargaData = nullptr;

	return true;
}

void Texture::Shutdown() {
	if (this->pTextureView) {
		this->pTextureView->Release();
		this->pTextureView = nullptr;
	}

	if (this->pTexture) {
		this->pTexture->Release();
		this->pTexture = nullptr;
	}

	if (this->pTargaData) {
		delete[] this->pTargaData;
		this->pTargaData = nullptr;
	}
}

ID3D11ShaderResourceView* Texture::GetTexture() {
	return this->pTextureView;
}

bool Texture::LoadTarga(const char* filename, int& height, int& width) {
	FILE* pFile;
	int error = fopen_s(&pFile, filename, "rb");
	if (error != 0) {
		printf("ERROR: error opening file, fopen_s returned %d\n", error);
		return false;
	}

	// Read the file header
	TargaHeader targaFileHeader;
	unsigned int readCount = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, pFile);
	if (readCount != 1) {
		printf("ERROR: Read in an unexpected number of elements on 'fread()': %d\n", readCount);
		return false;
	}

	height = (int)targaFileHeader.height;
	width = (int)targaFileHeader.width;
	int bpp = (int)targaFileHeader.bpp;

	// We are only accepting 32-bit texture files
	if (bpp != 32) {
		printf("ERROR: Read in an unexpected bpp from \"%s\": %d\n", filename, bpp);
		return false;
	}

	// Allocate memory for the raw targa data
	int imageSize = width * height * 4; // in bytes; each pixel has 4: r, g, b, a
	unsigned char* targaImageRawData = new unsigned char[imageSize];
	readCount = (unsigned int)fread(targaImageRawData, 1, imageSize, pFile);
	if (readCount != imageSize) {
		// we expect to have read # of bytes based on width * height * 4
		printf("ERROR: Read unexpected number of bytes %d. Expected %d.\n", readCount, imageSize);
		return false;
	}

	// We're done reading!
	error = fclose(pFile);
	if (error != 0) {
		printf("ERROR: error closing file, fclose returned %d\n", error);
		return false;
	}

	// Now we have to read the raw targa data backwards into our destination data array since targa
	// format stores the data upside down lmao
	this->pTargaData = new unsigned char[imageSize];

	int targaIndex = (height * width * 4) - (width * 4); // start at the end of the raw targa buffer
	for (int i = 0; i < height; i++) {
		int rowOffset = i * width;
		for (int j = 0; j < width * 4; j+=4) {
			this->pTargaData[j + rowOffset] = targaImageRawData[targaIndex + 2]; // red
			this->pTargaData[j + rowOffset + 1] = targaImageRawData[targaIndex + 1]; // green
			this->pTargaData[j + rowOffset + 2] = targaImageRawData[targaIndex + 0]; // blue
			this->pTargaData[j + rowOffset + 3] = targaImageRawData[targaIndex + 4]; // alpha

			targaIndex += 4;
		}
		targaIndex -= (width * 8);
	}

	// Allocate memory for the targa destination data.
	//this->pTargaData = new unsigned char[imageSize];

	//// Initialize the index into the targa destination data array.
	//int index = 0;

	//// Initialize the index into the targa image data.
	//int k = (width * height * 4) - (width * 4);

	//// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
	//for (int j = 0; j < height; j++)
	//{
	//	for (int i = 0; i < width; i++)
	//	{
	//		this->pTargaData[index + 0] = targaImageRawData[k + 2];  // Red.
	//		this->pTargaData[index + 1] = targaImageRawData[k + 1];  // Green.
	//		this->pTargaData[index + 2] = targaImageRawData[k + 0];  // Blue
	//		this->pTargaData[index + 3] = targaImageRawData[k + 3];  // Alpha

	//		// Increment the indexes into the targa data.
	//		k += 4;
	//		index += 4;
	//	}

	//	// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
	//	k -= (width * 8);
	//}

	delete[] targaImageRawData;
	targaImageRawData = nullptr;

	return true;
}