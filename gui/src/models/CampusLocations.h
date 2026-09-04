#ifndef CAMPUSLOCATIONS_H
#define CAMPUSLOCATIONS_H

#include <QStringList>

class CampusLocations
{
public:
    static QStringList predefinedLocations()
    {
        return {
            "Main Gate",
            "Central Library",
            "Main Cafeteria",
            "Academic Block",
            "Computer Lab",
            "Auditorium",
            "Sports Ground",
            "Hostel Block",
            "Administration Block",
            "Parking Area",
            "Student Activity Center"
        };
    }
};

#endif // CAMPUSLOCATIONS_H