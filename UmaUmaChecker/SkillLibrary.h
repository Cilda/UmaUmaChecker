#pragma once

#include "Skill.h"

class SkillLibrary
{
public:
	SkillLibrary();
	~SkillLibrary();

	bool Load();

public:
	std::vector<Skill> Skills;
};

