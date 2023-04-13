/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_MODDEF_H
#define _LIBCROC_MODDEF_H

#include <stdint.h>
#include "fixeddef.h"
#include "vecdef.h"

#define CROC_MODEL_BBOX_DIMS        9
#define CROC_MODEL_FACE_SIZE        84
#define CROC_MODEL_FACE_NAME_LENGTH 64
#define CROC_MODEL_COLL_INFO_SIZE   44

typedef enum CrocModelType {
    /*
     * Attempt to autodetect the model format.
     */
    CROC_MODEL_TYPE_AUTODETECT = -1,
    /*
     * Normal PC models. These have face material names.
     */
    CROC_MODEL_TYPE_NORMAL     =  0,
    /*
     * Model is a PlayStation 1 variant. These are identical
     * except without face materials. The material_id field will
     * be meaningful for these models.
     */
    CROC_MODEL_TYPE_PSX        =  1,
    /*
     * The tk%02u_trk.mod files. These are in PSX format even
     * in the PC version.
     */
    CROC_MODEL_TYPE_CAMERA     =  CROC_MODEL_TYPE_PSX,
} CrocModelType;

typedef enum CrocModelFlags {
    /* Model is used for collision. */
    CROC_MODEL_COLLISION = (1 << 0)
} CrocModelFlags;

typedef enum CrocModelFaceFlags {
    /*
     * Face is textured. If not set, then the {r,g,b}
     * fields are meaningful.
     */
    CROC_MODEL_FACE_TEXTURED = (1 << 0),
    /* Face is a quad and needs to be triangulated. */
    CROC_MODEL_FACE_QUAD     = (1 << 3),
    /* Face UVs need to be flipped. */
    CROC_MODEL_FACE_UV_FLIP  = (1 << 4)
} CrocModelFaceFlags;

typedef enum CrocModelUV {
    CROC_MODEL_UV_U0_V0 = 0,
    CROC_MODEL_UV_U0_V1 = 1,
    CROC_MODEL_UV_U1_V0 = 2,
    CROC_MODEL_UV_U1_V1 = 3,
} CrocModelUV;

typedef struct CrocModelFace
{
    /* Material name. Doesn't appear in camera collision models. */
    char        material[CROC_MODEL_FACE_NAME_LENGTH];
    /* Face normal, that's nice of them. */
    CrocVector  normal;
    /*
     * Face vertex indices. If CROC_MODEL_FACE_QUAD is set, then
     * all four elements are used. If not, then only three.
     */
    uint16_t    indices[4];

    /*
     * Not sure about this. This is unique for each
     * material, so it seems to be an ID.
     */
    uint16_t    material_id;
    uint8_t     r, g, b;
    uint8_t     flags;

    /* Triangulated indices and UVs in CCW winding-order. For convenience. */
    uint16_t    _idx_tri[6];
    CrocModelUV _uvs_tri[6];

    /* The UVs for each vertex. For convenience. */
    CrocModelUV _uvs[4];
} CrocModelFace;

typedef struct CrocModel
{
    /* I *think* this is the collision radius. */
    croc_x2012_t    radius;
    /*
     * Bounding Box:
     * Indices:
     *   7----8
     *  /|   /|
     * 3----4 |
     * | 5--|-6
     * |/   |/
     * 1----2
     *
     * 0 = centre.
     */
    CrocVector      bounding_box[CROC_MODEL_BBOX_DIMS];
    uint32_t        num_vertices;
    CrocVector      *vertices;
    CrocVector      *normals;
    uint32_t        num_faces;
    CrocModelFace   *faces;
} CrocModel;

#endif /* _LIBCROC_MODDEF_H */
