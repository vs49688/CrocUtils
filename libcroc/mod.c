/*
 * CrocUtils - Copyright (C) 2020 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, and only
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <vsclib.h>
#include <libcroc/mod.h>
#include <libcroc/vec.h>
#include <libcroc/fixed.h>

static void process_face_quad(CrocModelFace *f)
{
    assert(f->flags & CROC_MODEL_FACE_QUAD);

    f->_idx_tri[0] = f->indices[1];
    f->_idx_tri[1] = f->indices[0];
    f->_idx_tri[2] = f->indices[2];

    f->_idx_tri[3] = f->indices[2];
    f->_idx_tri[4] = f->indices[3];
    f->_idx_tri[5] = f->indices[1];

    if(f->flags & CROC_MODEL_FACE_UV_FLIP) {
        f->_uvs[0] = CROC_MODEL_UV_U0_V1;
        f->_uvs[1] = CROC_MODEL_UV_U1_V1;
        f->_uvs[2] = CROC_MODEL_UV_U0_V0;
        f->_uvs[3] = CROC_MODEL_UV_U1_V0;
    } else {
        f->_uvs[0] = CROC_MODEL_UV_U1_V1;
        f->_uvs[1] = CROC_MODEL_UV_U0_V1;
        f->_uvs[2] = CROC_MODEL_UV_U1_V0;
        f->_uvs[3] = CROC_MODEL_UV_U0_V0;
    }

    f->_uvs_tri[0] = f->_uvs[1];
    f->_uvs_tri[1] = f->_uvs[0];
    f->_uvs_tri[2] = f->_uvs[2];
    f->_uvs_tri[3] = f->_uvs[2];
    f->_uvs_tri[4] = f->_uvs[3];
    f->_uvs_tri[5] = f->_uvs[1];
}

static void process_face_tri(CrocModelFace *f)
{
    assert(!(f->flags & CROC_MODEL_FACE_QUAD));

    f->_idx_tri[0] = f->indices[2];
    f->_idx_tri[1] = f->indices[1];
    f->_idx_tri[2] = f->indices[0];

    if(f->flags & CROC_MODEL_FACE_UV_FLIP) {
        f->_uvs[0] = CROC_MODEL_UV_U1_V1;
        f->_uvs[1] = CROC_MODEL_UV_U0_V1;
        f->_uvs[2] = CROC_MODEL_UV_U0_V0;
    } else {
        f->_uvs[0] = CROC_MODEL_UV_U0_V0;
        f->_uvs[1] = CROC_MODEL_UV_U0_V1;
        f->_uvs[2] = CROC_MODEL_UV_U1_V1;
    }

    f->_uvs_tri[0] = f->_uvs[2];
    f->_uvs_tri[1] = f->_uvs[1];
    f->_uvs_tri[2] = f->_uvs[0];
}

static int read_face(FILE *f, CrocModelFace *face, int cam)
{
    uint8_t buf[CROC_MODEL_FACE_SIZE] = { 0 };
    size_t size = CROC_MODEL_FACE_SIZE;
    uint8_t *start = buf;

    if(cam) {
        size  -= CROC_MODEL_FACE_NAME_LENGTH;
        start += CROC_MODEL_FACE_NAME_LENGTH;
    }

    if(fread(start, size, 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    if(!cam)
        memcpy(face->material, buf + 0, sizeof(face->material));

    croc_vector_read(buf + 64, &face->normal);

    face->indices[0] = vsc_read_leu16(buf + 72);
    face->indices[1] = vsc_read_leu16(buf + 74);
    face->indices[2] = vsc_read_leu16(buf + 76);
    face->indices[3] = vsc_read_leu16(buf + 78);

    face->flags      = vsc_read_uint8(buf + 83);

    if(face->flags & CROC_MODEL_FACE_TEXTURED) {
        face->material_id = vsc_read_leu16(buf + 80);
        face->r           = 0;
        face->g           = 0;
        face->b           = 0;
    } else {
        face->material_id = 0;
        face->r           = vsc_read_uint8(buf + 80);
        face->g           = vsc_read_uint8(buf + 81);
        face->b           = vsc_read_uint8(buf + 82);
    }

    if(face->flags & CROC_MODEL_FACE_QUAD)
        process_face_quad(face);
    else
        process_face_tri(face);

    return 0;
}

static int croc_mod_read_submodel(FILE *f, CrocModel *m, uint16_t flags)
{
    m->radius.v = vsc_fread_leu32(f);
    for(int i = 0; i < CROC_MODEL_BBOX_DIMS; ++i) {
        m->bounding_box[i].x.v   = vsc_fread_leu16(f);
        m->bounding_box[i].y.v   = vsc_fread_leu16(f);
        m->bounding_box[i].z.v   = vsc_fread_leu16(f);
        m->bounding_box[i].pad.v = vsc_fread_leu16(f);
    }

    m->num_vertices = vsc_fread_leu32(f);

    if(feof(f) || ferror(f)) {
        errno = EIO;
        return -1;
    }

    if(m->num_vertices > UINT16_MAX) {
        errno = ERANGE;
        return -1;
    }

    if(!(m->vertices = calloc(m->num_vertices, sizeof(CrocVector))))
        return -1;

    if(!(m->normals = calloc(m->num_vertices, sizeof(CrocVector))))
        return -1;

    for(uint32_t i = 0; i < m->num_vertices; ++i) {
        if(croc_vector_fread(f, m->vertices + i) < 0)
            return -1;
    }

    for(uint32_t i = 0; i < m->num_vertices; ++i) {
        if(croc_vector_fread(f, m->normals + i) < 0)
            return -1;
    }

    m->num_faces = vsc_fread_leu32(f);
    if(feof(f) || ferror(f)) {
        errno = EIO;
        return -1;
    }

    if(m->num_faces > UINT16_MAX) {
        errno = ERANGE;
        return -1;
    }

    if(!(m->faces = calloc(m->num_faces, sizeof(CrocModelFace))))
        return -1;

    for(uint32_t i = 0; i < m->num_faces; ++i) {
        if(read_face(f, m->faces + i, 0) < 0)
            return -1;
    }

    if(flags & CROC_MODEL_COLLISION) {
        /*
         * Not sure what the rest of this is, I think it's something
         * to do with collision. Weird things happened when I changed
         * things. Further investigation is required. Just skip it for now.
         *
         * It's 44 bytes * sum of the first two uint16's.
         */
        uint16_t x = vsc_fread_leu16(f);
        uint16_t y = vsc_fread_leu16(f);

        if(feof(f) || ferror(f)) {
            errno = EIO;
            return -1;
        }

        if(fseek(f, CROC_MODEL_COLL_INFO_SIZE * (x + y), SEEK_CUR) < 0)
            return -1;
    }
    return 0;
}

int croc_mod_read_many(FILE *f, CrocModel **models, size_t *num)
{
    uint16_t _num, flags;
    int err;
    CrocModel *_models;

    _num  = vsc_fread_leu16(f);
    flags = vsc_fread_leu16(f);

    if(feof(f) || ferror(f)) {
        errno = EIO;
        return -1;
    }

    /* Sanity check this. */
    if(_num > 128) {
        errno = EINVAL;
        return -1;
    }

    if((_models = calloc(_num, sizeof(CrocModel))) == NULL) {
        errno = ENOMEM;
        return -1;
    }

    for(int i = 0; i < _num; ++i)
        croc_mod_init(_models + i);

    for(int i = 0; i < _num; ++i) {
        if(croc_mod_read_submodel(f, _models + i, flags) < 0)
            goto fail;
    }

    *models = _models;
    *num = _num;
    return 0;

fail:
    err = errno;
    croc_mod_free_many(_models, _num);
    errno = err;
    return -1;
}

void croc_mod_free(CrocModel *m)
{
    if(m == NULL)
        return;

    if(m->faces != NULL)
        free(m->faces);

    if(m->normals != NULL)
        free(m->normals);

    if(m->vertices != NULL)
        free(m->vertices);

    croc_mod_init(m);
}

void croc_mod_free_many(CrocModel *m, size_t count)
{
    if(m == NULL)
        return;

    for(size_t i = 0; i < count; ++i)
        croc_mod_free(m + i);

    free(m);
}

CrocModel *croc_mod_init(CrocModel *m)
{
    if(m == NULL)
        return NULL;

    memset(m, 0, sizeof(CrocModel));

    return m;
}

int croc_mod_validate(const CrocModel *m, void(*proc)(void *, const char*, ...), void *user)
{
    int bad = 0;

    if(m == NULL) {
        errno = EINVAL;
        return -1;
    }

    for(uint32_t i = 0; i < m->num_faces; ++i) {
        CrocModelFace *f = m->faces + i;
        int nidx = (f->flags & CROC_MODEL_FACE_QUAD) ? 4 : 3;

        for(int v = 0; v < nidx; ++v) {
            if(f->indices[v] < m->num_vertices)
                continue;

            ++bad;

            if(proc == NULL)
                continue;

            proc(user, "face %u, vertex %d out of range (%u >= %u)\n",
                 i, v, f->indices[v], m->num_vertices);
        }
    }

    return bad ? -1 : 0;
}

static int facesort(const void *a, const void *b)
{
    const CrocModelFace *f1 = a;
    const CrocModelFace *f2 = b;
    return strncmp(f1->material, f2->material, sizeof(f1->material));
}

void croc_mod_sort_faces(CrocModel *m)
{
    qsort(m->faces, m->num_faces, sizeof(CrocModelFace), facesort);
}

int croc_mod_write_obj(FILE *f, const CrocModel *m)
{
    const char *lastmat = "";

    for(uint32_t i = 0; i < m->num_vertices; ++i) {
        fprintf(f, "v  %f %f %f\n",
            croc_x0412_to_float(m->vertices[i].x),
            croc_x0412_to_float(m->vertices[i].y),
            croc_x0412_to_float(m->vertices[i].z)
        );

        fprintf(f, "vn %f %f %f\n",
            croc_x0412_to_float(m->normals[i].x),
            croc_x0412_to_float(m->normals[i].y),
            croc_x0412_to_float(m->normals[i].z)
        );
    }

    /*
     * Make sure these match the order of CrocModelUV.
     * It's nice that OBJ allows this, otherwise I'd have to
     * duplicate vertices.
     */
    fprintf(f,
        "vt 0 0\n" /* CROC_MODEL_UV_U0_V0 */
        "vt 0 1\n" /* CROC_MODEL_UV_U0_V1 */
        "vt 1 0\n" /* CROC_MODEL_UV_U1_V0 */
        "vt 1 1\n" /* CROC_MODEL_UV_U1_V1 */
    );

    for(uint32_t i = 0; i < m->num_faces; ++i) {
        const CrocModelFace *face = m->faces + i;

        /*
         * From the OBJ/MTL spec:
         *   "If a material name is not specified, a white material is used."
         * Also don't print duplicate usemtl's if we can help it.
         *
         * NB: These are commented out until proper material handling is implemented.
         */
        if(strncmp(lastmat, face->material, sizeof(face->material)) != 0) {
            fprintf(f, "# usemtl %.*s\n", (int)strnlen(face->material, sizeof(face->material)), face->material);
            lastmat = face->material;
        }

        fprintf(f, "f  %u/%d/%u %u/%d/%u %u/%d/%u\n",
            face->_idx_tri[0] + 1, (int)face->_uvs_tri[0] + 1, face->_idx_tri[0] + 1,
            face->_idx_tri[1] + 1, (int)face->_uvs_tri[1] + 1, face->_idx_tri[1] + 1,
            face->_idx_tri[2] + 1, (int)face->_uvs_tri[2] + 1, face->_idx_tri[2] + 1
        );

        if(face->flags & CROC_MODEL_FACE_QUAD) {
            fprintf(f, "f  %u/%d/%u %u/%d/%u %u/%d/%u\n",
                face->_idx_tri[3] + 1, (int)face->_uvs_tri[3] + 1, face->_idx_tri[3] + 1,
                face->_idx_tri[4] + 1, (int)face->_uvs_tri[4] + 1, face->_idx_tri[4] + 1,
                face->_idx_tri[5] + 1, (int)face->_uvs_tri[5] + 1, face->_idx_tri[5] + 1
            );
        }
    }

    if(ferror(f)) {
        errno = EIO;
        return -1;
    }

    return 0;
}
