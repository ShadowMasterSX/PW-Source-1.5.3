#include "db.h"

DB DB::instance;

std::string CheatItem::XmlString() const
{
	return "";
}

bool DB::Write(const std::vector<CheatItem> &cheats)
{
	return true;
}
