#pragma once

class Config
{
public:
	Config();
	virtual ~Config();

	bool Load();
	void Create();

public:
	int WindowX = 0;
	int WindowY = 0;
};

