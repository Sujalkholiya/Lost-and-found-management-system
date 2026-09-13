#ifndef CLAIM_CONTROLLER_H
#define CLAIM_CONTROLLER_H

#include <crow.h>

crow::response createClaim(const crow::request& req, int itemId);
crow::response getUserClaims(const crow::request& req);
crow::response getMyClaimForItem(const crow::request& req, int itemId);
crow::response getClaimById(const crow::request& req, int id);
crow::response getAdminClaims(const crow::request& req);
crow::response approveClaim(const crow::request& req, int id);
crow::response rejectClaim(const crow::request& req, int id);
crow::response handoverClaim(const crow::request& req, int id);

#endif // CLAIM_CONTROLLER_H

