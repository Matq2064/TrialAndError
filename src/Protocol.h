//
// Created by Matq on 04/06/2023.
//

#ifndef TRIALANDERROR_SRC_PROTOCOL_H_
#define TRIALANDERROR_SRC_PROTOCOL_H_

enum EntityFormFactor {
    ENTFORM_NORMAL,
    ENTFORM_DIRECTIONAL
};

enum EntityType {
    ENTTYPE_ERROR,
    ENTTYPE_AMMO,
    ENTTYPE_ITEM,
    ENTTYPE_CRATE,
    ENTTYPE_BULLET,
    ENTTYPE_CHARACTER,
    NUM_ENTTYPES
};

#endif //TRIALANDERROR_SRC_PROTOCOL_H_
