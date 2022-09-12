#pragma once

#include <d3d11.h>
#include <stdio.h>

class Texture
{
private:
	// This is the .tga file header structure
	struct TargaHeader {
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

	// Supporting other formats would just be other Load_() functions here.
	bool LoadTarga(const char*, int&, int&);

	unsigned char* pTargaData;
	ID3D11Texture2D* pTexture;
	ID3D11ShaderResourceView* pTextureView;

public:
	Texture();
	Texture(const Texture&);
	~Texture();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
};