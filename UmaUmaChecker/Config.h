#pragma once

class Config
{
public:
	Config();
	virtual ~Config();

	bool Load();
	void Create();
	void Save();

public:
	int WindowX = 0;
	int WindowY = 0;
};

