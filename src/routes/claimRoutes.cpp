#include <crow.h>
#include "../controllers/claimController.h"

void claimRoutes(crow::SimpleApp& app)
{
    // Submit claim for item
    CROW_ROUTE(app, "/api/items/<int>/claims")
    .methods(crow::HTTPMethod::POST)
    (
        createClaim
    );

    // Current user's claims
    CROW_ROUTE(app, "/api/users/me/claims")
    .methods(crow::HTTPMethod::GET)
    (
        getUserClaims
    );

    // Current user's claim status on a specific item
    CROW_ROUTE(app, "/api/items/<int>/my-claim")
    .methods(crow::HTTPMethod::GET)
    (
        getMyClaimForItem
    );

    // Specific claim detail
    CROW_ROUTE(app, "/api/claims/<int>")
    .methods(crow::HTTPMethod::GET)
    (
        getClaimById
    );

    // Admin view of all claims + stats
    CROW_ROUTE(app, "/api/admin/claims")
    .methods(crow::HTTPMethod::GET)
    (
        getAdminClaims
    );

    // Admin approve claim
    CROW_ROUTE(app, "/api/admin/claims/<int>/approve")
    .methods(crow::HTTPMethod::PUT)
    (
        approveClaim
    );

    // Admin reject claim
    CROW_ROUTE(app, "/api/admin/claims/<int>/reject")
    .methods(crow::HTTPMethod::PUT)
    (
        rejectClaim
    );

    // Admin mark claim as handed over
    CROW_ROUTE(app, "/api/admin/claims/<int>/handover")
    .methods(crow::HTTPMethod::PUT)
    (
        handoverClaim
    );
}

