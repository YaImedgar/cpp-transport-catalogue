#include "geo.h"

bool tc::geo::Coordinates::operator==(const Coordinates& other) const
{
	return lat == other.lat && lng == other.lng;
}

bool tc::geo::Coordinates::operator!=(const Coordinates& other) const
{
	return !(*this == other);
}