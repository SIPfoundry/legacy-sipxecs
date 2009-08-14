//-< TTREE.CPP >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  6-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "ttree.h"

BEGIN_FASTDB_NAMESPACE

#ifdef USE_LOCALE_SETTINGS
#ifdef IGNORE_CASE
#define strcmp(x, y) stricoll(x, y)
#define strncmp(x, y, n) strincmp(x, y, n)
#else
#define strcmp(x, y) strcoll(x, y)
#endif
#else
#ifdef IGNORE_CASE
#define strcmp(x, y) stricmp(x, y)
#define strncmp(x, y, n) strincmp(x, y, n)
#endif
#endif

inline int keycmp(void* p, void* q, int type, int sizeofType, dbUDTComparator comparator)
{
    switch (type) {
      case dbField::tpBool:
        return *(bool*)p - *(bool*)q;
      case dbField::tpInt1:
        return *(int1*)p - *(int1*)q;
      case dbField::tpInt2:
        return *(int2*)p - *(int2*)q;
      case dbField::tpInt4:
        return *(int4*)p < *(int4*)q ? -1 : *(int4*)p == *(int4*)q ? 0 : 1;
      case dbField::tpInt8:
        return *(db_int8*)p < *(db_int8*)q ? -1 : *(db_int8*)p == *(db_int8*)q ? 0 : 1;
      case dbField::tpReference:
        return *(oid_t*)p < *(oid_t*)q ? -1 : *(oid_t*)p == *(oid_t*)q ? 0 : 1;
      case dbField::tpReal4:
        return *(real4*)p < *(real4*)q ? -1 : *(real4*)p == *(real4*)q ? 0 : 1;
      case dbField::tpReal8:
        return *(real8*)p < *(real8*)q ? -1 : *(real8*)p == *(real8*)q ? 0 : 1;
      case dbField::tpRawBinary:
        return comparator(p, q, sizeofType);
      default:
        assert(false);
        return 0;
    }
}

void dbTtree::find(dbDatabase* db, oid_t treeId, dbSearchContext& sc)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->find(db, sc);
    }
}

void dbTtree::prefixSearch(dbDatabase* db, oid_t treeId, dbSearchContext& sc)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->prefixSearch(db, sc);
    }
}

oid_t dbTtree::allocate(dbDatabase* db)
{
    oid_t oid = db->allocateObject(dbTtreeMarker);
    dbTtree* tree = (dbTtree*)db->get(oid);
    tree->root = 0;
    return oid;
}

void dbTtree::insert(dbDatabase* db, oid_t treeId, oid_t recordId,
                     int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId == 0) {
        oid_t nodeId = dbTtreeNode::allocate(db, recordId);
        ((dbTtree*)db->put(treeId))->root = nodeId;
    } else {
        byte* rec = (byte*)db->getRow(recordId);
        byte* key = rec + offs;
        if (type == dbField::tpString) {
            key = rec + ((dbVarying*)key)->offs;
        }
        oid_t nodeId = rootId;
        dbTtreeNode::insert(db, nodeId, recordId, key, type, sizeofType, comparator, offs);
        if (rootId != nodeId) {
            ((dbTtree*)db->put(treeId))->root = nodeId;
        }
    }
}


void dbTtree::remove(dbDatabase* db, oid_t treeId, oid_t recordId,
                     int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    byte* rec = (byte*)db->getRow(recordId);
    byte* key = rec + offs;
    if (type == dbField::tpString) {
        key = rec + ((dbVarying*)key)->offs;
    }
    oid_t nodeId = rootId;
    int h = dbTtreeNode::remove(db, nodeId, recordId, key, type, sizeofType, comparator, offs);
    assert(h >= 0);
    if (nodeId != rootId) {
        ((dbTtree*)db->put(treeId))->root = nodeId;
    }
}


void dbTtree::purge(dbDatabase* db, oid_t treeId)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    dbTtreeNode::purge(db, rootId);
    ((dbTtree*)db->put(treeId))->root = 0;
}

void dbTtree::drop(dbDatabase* db, oid_t treeId)
{
    purge(db, treeId);
    db->freeObject(treeId);
}


void dbTtree::traverseForward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->traverseForward(db, cursor);
    }
}

void dbTtree::traverseBackward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->traverseBackward(db, cursor);
    }
}


void dbTtree::traverseForward(dbDatabase* db, oid_t treeId,
                             dbAnyCursor* cursor, dbExprNode* condition)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->traverseForward(db, cursor,condition);
    }
}


void dbTtree::traverseBackward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor, dbExprNode* condition)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) {
        ((dbTtreeNode*)db->get(rootId))->traverseBackward(db,cursor,condition);
    }
}

static inline int prefcmp(char const* key, char const* prefix) {
    return strncmp(key, prefix, strlen(prefix));
}

bool dbTtreeNode::prefixSearch(dbDatabase* db, dbSearchContext& sc)
{
    char* rec;
    int   l, r, m, n = nItems;

    sc.probes += 1;
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);

    assert (sc.type == dbField::tpString);
    rec = (char*)db->getRow(item[0]);
    char* key = sc.firstKey;
    if (prefcmp(key, rec+((dbVarying*)(rec+sc.offs))->offs) > 0) {
        rec = (char*)db->getRow(item[n-1]);
        if (prefcmp(key, rec+((dbVarying*)(rec+sc.offs))->offs) > 0) {
            if (right != 0) {
                return ((dbTtreeNode*)db->get(right))->find(db, sc);
            }
            return true;
        }
        for (l = 0, r = n; l < r;) {
            m = (l + r) >> 1;
            rec = (char*)db->getRow(item[m]);
            if (prefcmp(sc.firstKey, rec + ((dbVarying*)(rec+sc.offs))->offs) > 0) {
                l = m+1;
            } else {
                r = m;
            }
        }
        while (r < n) {
            rec = (char*)db->getRow(item[r]);
            if (prefcmp(key, rec + ((dbVarying*)(rec+sc.offs))->offs) < 0) {
                return false;
            }
            if (!sc.condition
                || db->evaluate(sc.condition, item[r], table, sc.cursor))
            {
                if (!sc.cursor->add(item[r])) {
                    return false;
                }
            }
            r += 1;
        }
        if (right != 0) {
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        }
        return true;
    }
    if (left != 0) {
        if (!((dbTtreeNode*)db->get(left))->find(db, sc)) {
            return false;
        }
    }
    for (l = 0; l < n; l++) {
        rec = (char*)db->getRow(item[l]);
        if (prefcmp(key, rec + ((dbVarying*)(rec+sc.offs))->offs) < 0) {
            return false;
        }
        if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
            if (!sc.cursor->add(item[l])) {
                return false;
            }
        }
    }
    if (right != 0) {
        return ((dbTtreeNode*)db->get(right))->find(db, sc);
    }
    return false;
}


bool dbTtreeNode::find(dbDatabase* db, dbSearchContext& sc)
{
    char* rec;
    int   diff;
    int   l, r, m, n = nItems;

    sc.probes += 1;
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);

    if (sc.type == dbField::tpString) {
        if (sc.firstKey != NULL) {
            rec = (char*)db->getRow(item[0]);
            diff = strcmp(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs);
            if (diff >= sc.firstKeyInclusion) {
                rec = (char*)db->getRow(item[n-1]);
                diff = strcmp(sc.firstKey,
                              rec+((dbVarying*)(rec+sc.offs))->offs);
                if (diff >= sc.firstKeyInclusion) {
                    if (right != 0) {
                        return ((dbTtreeNode*)db->get(right))->find(db, sc);
                    }
                    return true;
                }
                for (l = 0, r = n; l < r;) {
                    m = (l + r) >> 1;
                    rec = (char*)db->getRow(item[m]);
                    diff = strcmp(sc.firstKey,
                                  rec + ((dbVarying*)(rec+sc.offs))->offs);
                    if (diff >= sc.firstKeyInclusion) {
                        l = m+1;
                    } else {
                        r = m;
                    }
                }
                while (r < n) {
                    rec = (char*)db->getRow(item[r]);
                    if ((sc.lastKey != NULL
                         && strcmp(rec + ((dbVarying*)(rec+sc.offs))->offs,
                                   sc.lastKey) >= sc.lastKeyInclusion) ||
                        (sc.prefixLength != 0
                         && memcmp(rec + ((dbVarying*)(rec+sc.offs))->offs,
                                   sc.firstKey,
                                   sc.prefixLength) != 0))
                    {
                        return false;
                    }
                    if (!sc.condition
                        || db->evaluate(sc.condition, item[r], table, sc.cursor))
                    {
                        if (!sc.cursor->add(item[r])) {
                            return false;
                        }
                    }
                    r += 1;
                }
                if (right != 0) {
                    return ((dbTtreeNode*)db->get(right))->find(db, sc);
                }
                return true;
            }
        }
        if (left != 0) {
            if (!((dbTtreeNode*)db->get(left))->find(db, sc)) {
                return false;
            }
        }
        for (l = 0; l < n; l++) {
            rec = (char*)db->getRow(item[l]);
            if ((sc.lastKey != NULL
                 && strcmp(rec + ((dbVarying*)(rec+sc.offs))->offs,
                           sc.lastKey) >= sc.lastKeyInclusion) ||
                (sc.prefixLength != 0
                 && memcmp(rec + ((dbVarying*)(rec+sc.offs))->offs,
                           sc.firstKey,
                           sc.prefixLength) != 0))
            {
                return false;
            }
            if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
                if (!sc.cursor->add(item[l])) {
                    return false;
                }
            }
        }
        if (right != 0) {
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        }
    } else {
        if (sc.firstKey != NULL) {
            rec = (char*)db->getRow(item[0]);
            diff = keycmp(sc.firstKey, rec+sc.offs, sc.type, sc.sizeofType, sc.comparator);
            if (diff >= sc.firstKeyInclusion) {
                rec = (char*)db->getRow(item[n-1]);
                diff = keycmp(sc.firstKey, rec+sc.offs, sc.type, sc.sizeofType, sc.comparator);
                if (diff >= sc.firstKeyInclusion) {
                    if (right != 0) {
                        return ((dbTtreeNode*)db->get(right))->find(db, sc);
                    }
                    return true;
                }
                for (l = 0, r = n; l < r;) {
                    m = (l + r) >> 1;
                    rec = (char*)db->getRow(item[m]);
                    diff = keycmp(sc.firstKey, rec+sc.offs, sc.type, sc.sizeofType, sc.comparator);
                    if (diff >= sc.firstKeyInclusion) {
                        l = m+1;
                    } else {
                        r = m;
                    }
                }
                while (r < n) {
                    rec = (char*)db->getRow(item[r]);
                    if (sc.lastKey != NULL
                        && keycmp(rec+sc.offs, sc.lastKey, sc.type, sc.sizeofType, sc.comparator)
                           >= sc.lastKeyInclusion)
                    {
                        return false;
                    }
                    if (!sc.condition
                        || db->evaluate(sc.condition, item[r], table, sc.cursor))
                    {
                        if (!sc.cursor->add(item[r])) {
                            return false;
                        }
                    }
                    r += 1;
                }
                if (right != 0) {
                    return ((dbTtreeNode*)db->get(right))->find(db, sc);
                }
                return true;
            }
        }
        if (left != 0) {
            if (!((dbTtreeNode*)db->get(left))->find(db, sc)) {
                return false;
            }
        }
        for (l = 0; l < n; l++) {
            rec = (char*)db->getRow(item[l]);
            if (sc.lastKey != NULL && keycmp(rec+sc.offs, sc.lastKey, sc.type, sc.sizeofType, sc.comparator)
                >= sc.lastKeyInclusion)
            {
                return false;
            }
            if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
                if (!sc.cursor->add(item[l])) {
                    return false;
                }
            }
        }
        if (right != 0) {
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        }
    }
    return true;
}


oid_t dbTtreeNode::allocate(dbDatabase* db, oid_t recordId)
{
    oid_t nodeId = db->allocateObject(dbTtreeNodeMarker);
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    node->nItems = 1;
    node->item[0] = recordId;
    node->left = node->right = 0;
    node->balance = 0;
    return nodeId;
}

bool dbTtreeNode::insert(dbDatabase* db, oid_t& nodeId, oid_t recordId,
                         void* key, int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    char* rec = (char*)db->getRow(node->item[0]);
    int n = node->nItems;
    int diff = (type == dbField::tpString)
        ? strcmp((char*)key, rec + ((dbVarying*)(rec+offs))->offs)
        : keycmp(key, rec+offs, type, sizeofType, comparator);

    if (diff <= 0) {
        oid_t leftId = node->left;
        if ((leftId == 0 || diff == 0) && node->nItems != pageSize) {
            node = (dbTtreeNode*)db->put(nodeId);
            for (int i = n; i > 0; i--) node->item[i] = node->item[i-1];
            node->item[0] = recordId;
            node->nItems += 1;
            return false;
        }
        if (leftId == 0) {
            leftId = allocate(db, recordId);
            node = (dbTtreeNode*)db->put(nodeId);
            node->left = leftId;
        } else {
            oid_t childId = leftId;
            bool grow = insert(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != leftId) {
                ((dbTtreeNode*)db->put(nodeId))->left = leftId = childId;
            }
            if (!grow) return false;
        }
        node = (dbTtreeNode*)db->put(nodeId);
        if (node->balance > 0) {
            node->balance = 0;
            return false;
        } else if (node->balance == 0) {
            node->balance = -1;
            return true;
        } else {
            dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
            node = (dbTtreeNode*)db->get(nodeId);
            if (left->balance < 0) { // single LL turn
                node->left = left->right;
                left->right = nodeId;
                node->balance = 0;
                left->balance = 0;
                nodeId = leftId;
            } else { // double LR turn
                oid_t rightId = left->right;
                dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
                left = (dbTtreeNode*)db->get(leftId);
                node = (dbTtreeNode*)db->get(nodeId);
                left->right = right->left;
                right->left = leftId;
                node->left = right->right;
                right->right = nodeId;
                node->balance = (right->balance < 0) ? 1 : 0;
                left->balance = (right->balance > 0) ? -1 : 0;
                right->balance = 0;
                nodeId = rightId;
            }
            return false;
        }
    }
    rec = (char*)db->getRow(node->item[n-1]);
    diff = (type == dbField::tpString)
        ? strcmp((char*)key, rec + ((dbVarying*)(rec+offs))->offs)
        : keycmp(key, rec+offs, type, sizeofType, comparator);
    if (diff >= 0) {
        oid_t rightId = node->right;
        if ((rightId == 0 || diff == 0) && node->nItems != pageSize) {
            node = (dbTtreeNode*)db->put(nodeId);
            node->item[n] = recordId;
            node->nItems += 1;
            return false;
        }
        if (rightId == 0) {
            rightId = allocate(db, recordId);
            node = (dbTtreeNode*)db->put(nodeId);
            node->right = rightId;
        } else {
            oid_t childId = rightId;
            bool grow = insert(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != rightId) {
                ((dbTtreeNode*)db->put(nodeId))->right = rightId = childId;
            }
            if (!grow) return false;
        }
        node = (dbTtreeNode*)db->put(nodeId);
        if (node->balance < 0) {
            node->balance = 0;
            return false;
        } else if (node->balance == 0) {
            node->balance = 1;
            return true;
        } else {
            dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
            node = (dbTtreeNode*)db->get(nodeId);
            if (right->balance > 0) { // single RR turn
                node->right = right->left;
                right->left = nodeId;
                node->balance = 0;
                right->balance = 0;
                nodeId = rightId;
            } else { // double RL turn
                oid_t leftId = right->left;
                dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
                right = (dbTtreeNode*)db->get(rightId);
                node = (dbTtreeNode*)db->get(nodeId);
                right->left = left->right;
                left->right = rightId;
                node->right = left->left;
                left->left = nodeId;
                node->balance = (left->balance > 0) ? -1 : 0;
                right->balance = (left->balance < 0) ? 1 : 0;
                left->balance = 0;
                nodeId = leftId;
            }
            return false;
        }
    }
    int l = 1, r = n-1;
    if (type == dbField::tpString) {
        while (l < r)  {
            int i = (l+r) >> 1;
            rec = (char*)db->getRow(node->item[i]);
            diff = strcmp((char*)key, rec + ((dbVarying*)(rec+offs))->offs);
            if (diff > 0) {
                l = i + 1;
            } else {
                r = i;
                if (diff == 0) {
                    break;
                }
            }
        }
    } else {
        while (l < r)  {
            int i = (l+r) >> 1;
            rec = (char*)db->getRow(node->item[i]);
            diff = keycmp(key, rec+offs, type, sizeofType, comparator);
            if (diff > 0) {
                l = i + 1;
            } else {
                r = i;
                if (diff == 0) {
                    break;
                }
            }
        }
    }
    // Insert before item[r]
    node = (dbTtreeNode*)db->put(nodeId);
    if (n != pageSize) {
        for (int i = n; i > r; i--) node->item[i] = node->item[i-1];
        node->item[r] = recordId;
        node->nItems += 1;
        return false;
    } else {
        oid_t reinsertId;
        if (node->balance >= 0) {
            reinsertId = node->item[0];
            for (int i = 1; i < r; i++) node->item[i-1] = node->item[i];
            node->item[r-1] = recordId;
        } else {
            reinsertId = node->item[n-1];
            for (int i = n-1; i > r; i--) node->item[i] = node->item[i-1];
            node->item[r] = recordId;
        }
        rec = (char*)db->getRow(reinsertId);
        key = rec + offs;
        if (type == dbField::tpString) {
            key = rec + ((dbVarying*)key)->offs;
        }
        return insert(db, nodeId, reinsertId, key, type, sizeofType, comparator, offs);
    }
}

inline int dbTtreeNode::balanceLeftBranch(dbDatabase* db, oid_t& nodeId)
{
    dbTtreeNode* node = (dbTtreeNode*)db->put(nodeId);
    if (node->balance < 0) {
        node->balance = 0;
        return 1;
    } else if (node->balance == 0) {
        node->balance = 1;
        return 0;
    } else {
        oid_t rightId = node->right;
        dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
        node = (dbTtreeNode*)db->get(nodeId);
        if (right->balance >= 0) { // single RR turn
            node->right = right->left;
            right->left = nodeId;
            if (right->balance == 0) {
                node->balance = 1;
                right->balance = -1;
                nodeId = rightId;
                return 0;
            } else {
                node->balance = 0;
                right->balance = 0;
                nodeId = rightId;
                return 1;
            }
        } else { // double RL turn
            oid_t leftId = right->left;
            dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
            node = (dbTtreeNode*)db->get(nodeId);
            right = (dbTtreeNode*)db->get(rightId);
            right->left = left->right;
            left->right = rightId;
            node->right = left->left;
            left->left = nodeId;
            node->balance = left->balance > 0 ? -1 : 0;
            right->balance = left->balance < 0 ? 1 : 0;
            left->balance = 0;
            nodeId = leftId;
            return 1;
        }
    }
}


inline int dbTtreeNode::balanceRightBranch(dbDatabase* db, oid_t& nodeId)
{
    dbTtreeNode* node = (dbTtreeNode*)db->put(nodeId);
    if (node->balance > 0) {
        node->balance = 0;
        return 1;
    } else if (node->balance == 0) {
        node->balance = -1;
        return 0;
    } else {
        oid_t leftId = node->left;
        dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
        node = (dbTtreeNode*)db->get(nodeId);
        if (left->balance <= 0) { // single LL turn
            node->left = left->right;
            left->right = nodeId;
            if (left->balance == 0) {
                node->balance = -1;
                left->balance = 1;
                nodeId = leftId;
                return 0;
            } else {
                node->balance = 0;
                left->balance = 0;
                nodeId = leftId;
                return 1;
            }
        } else { // double LR turn
            oid_t rightId = left->right;
            dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
            node = (dbTtreeNode*)db->get(nodeId);
            left = (dbTtreeNode*)db->get(leftId);
            left->right = right->left;
            right->left = leftId;
            node->left = right->right;
            right->right = nodeId;
            node->balance = right->balance < 0 ? 1 : 0;
            left->balance = right->balance > 0 ? -1 : 0;
            right->balance = 0;
            nodeId = rightId;
            return 1;
        }
    }
}

int dbTtreeNode::remove(dbDatabase* db, oid_t& nodeId, oid_t recordId,
                        void* key, int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    char* rec = (char*)db->getRow(node->item[0]);
    int n = node->nItems;
    int diff = (type == dbField::tpString)
             ? strcmp((char*)key, rec + ((dbVarying*)(rec+offs))->offs)
             : keycmp(key, rec+offs, type, sizeofType, comparator);
    if (diff <= 0) {
        oid_t leftId = node->left;
        if (leftId != 0) {
            oid_t childId = leftId;
            int h = remove(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != leftId) {
                ((dbTtreeNode*)db->put(nodeId))->left = childId;
            }
            if (h > 0) {
                return balanceLeftBranch(db, nodeId);
            } else if (h == 0) {
                return 0;
            }
        }
        assert (diff == 0);
    }
    rec = (char*)db->getRow(node->item[n-1]);
    diff = (type == dbField::tpString)
        ? strcmp((char*)key, rec + ((dbVarying*)(rec+offs))->offs)
        : keycmp(key, rec+offs, type, sizeofType, comparator);
    if (diff <= 0) {
        for (int i = 0; i < n; i++) {
            if (node->item[i] == recordId) {
                if (n == 1) {
                    if (node->right == 0) {
                        db->freeObject(nodeId);
                        nodeId = node->left;
                        return 1;
                    } else if (node->left == 0) {
                        db->freeObject(nodeId);
                        nodeId = node->right;
                        return 1;
                    }
                }
                node = (dbTtreeNode*)db->put(nodeId);
                oid_t leftId = node->left, rightId = node->right;
                if (n <= minItems) {
                    if (leftId != 0 && node->balance <= 0) {
                        dbTtreeNode* left = (dbTtreeNode*)db->get(leftId);
                        while (left->right != 0) {
                            left = (dbTtreeNode*)db->get(left->right);
                        }
                        while (--i >= 0) {
                            node->item[i+1] = node->item[i];
                        }
                        node->item[0] = left->item[left->nItems-1];
                        rec = (char*)db->getRow(node->item[0]);
                        key = rec + offs;
                        if (type == dbField::tpString) {
                            key = rec + ((dbVarying*)key)->offs;
                        }
                        oid_t childId = leftId;
                        int h = remove(db, childId, node->item[0],
                                       key, type, sizeofType, comparator, offs);
                        if (childId != leftId) {
                            ((dbTtreeNode*)db->get(nodeId))->left = childId;
                        }
                        if (h > 0) {
                            h = balanceLeftBranch(db, nodeId);
                        }
                        return h;
                    } else if (node->right != 0) {
                        dbTtreeNode* right = (dbTtreeNode*)db->get(rightId);
                        while (right->left != 0) {
                            right = (dbTtreeNode*)db->get(right->left);
                        }
                        while (++i < n) {
                            node->item[i-1] = node->item[i];
                        }
                        node->item[n-1] = right->item[0];
                        rec = (char*)db->getRow(node->item[n-1]);
                        key = rec + offs;
                        if (type == dbField::tpString) {
                            key = rec + ((dbVarying*)key)->offs;
                        }
                        oid_t childId = rightId;
                        int h = remove(db, childId, node->item[n-1],
                                       key, type, sizeofType, comparator, offs);
                        if (childId != rightId) {
                            ((dbTtreeNode*)db->get(nodeId))->right = childId;
                        }
                        if (h > 0) {
                            h = balanceRightBranch(db, nodeId);
                        }
                        return h;
                    }
                }
                while (++i < n) {
                    node->item[i-1] = node->item[i];
                }
                node->nItems -= 1;
                return 0;
            }
        }
    }
    oid_t rightId = node->right;
    if (rightId != 0) {
        oid_t childId = rightId;
        int h = remove(db, childId, recordId, key, type, sizeofType, comparator, offs);
        if (childId != rightId) {
            ((dbTtreeNode*)db->put(nodeId))->right = childId;
        }
        if (h > 0) {
            return balanceRightBranch(db, nodeId);
        } else {
            return h;
        }
    }
    return -1;
}

void dbTtreeNode::purge(dbDatabase* db, oid_t nodeId)
{
    if (nodeId != 0) {
        dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
        oid_t leftId = node->left;
        oid_t rightId = node->right;
        db->freeObject(nodeId);
        purge(db, leftId);
        purge(db, rightId);
    }
}

bool dbTtreeNode::traverseForward(dbDatabase* db, dbAnyCursor* cursor)
{
    if (left != 0) {
        if (!((dbTtreeNode*)db->get(left))->traverseForward(db, cursor)) {
            return false;
        }
    }
    for (int i = 0, n = nItems; i < n; i++) {
        if (!cursor->add(item[i])) {
            return false;
        }
    }
    if (right != 0) {
        return ((dbTtreeNode*)db->get(right))->traverseForward(db, cursor);
    }
    return true;
}

bool dbTtreeNode::traverseBackward(dbDatabase* db, dbAnyCursor* cursor)
{
    if (right != 0) {
        if (!((dbTtreeNode*)db->get(right))->traverseBackward(db, cursor)) {
            return false;
        }
    }
    for (int i = nItems; --i >= 0;) {
        if (!cursor->add(item[i])) {
            return false;
        }
    }
    if (left != 0) {
        return ((dbTtreeNode*)db->get(left))->traverseBackward(db, cursor);
    }
    return true;
}

bool dbTtreeNode::traverseForward(dbDatabase* db, dbAnyCursor* cursor,
                                  dbExprNode* condition)
{
    if (left != 0) {
        if (!((dbTtreeNode*)db->get(left))->traverseForward(db, cursor,
                                                            condition))
        {
            return false;
        }
    }
    dbTable* table = (dbTable*)db->getRow(cursor->table->tableId);
    for (int i = 0, n = nItems; i < n; i++) {
        if (db->evaluate(condition, item[i], table, cursor)) {
            if (!cursor->add(item[i])) {
                return false;
            }
        }
    }
    if (right != 0) {
        return ((dbTtreeNode*)db->get(right))->traverseForward(db, cursor,
                                                               condition);
    }
    return true;
}

bool dbTtreeNode::traverseBackward(dbDatabase* db, dbAnyCursor* cursor,
                                   dbExprNode* condition)
{
    if (right != 0) {
        if (!((dbTtreeNode*)db->get(right))->traverseBackward(db, cursor,
                                                             condition))
        {
            return false;
        }
    }
    dbTable* table = (dbTable*)db->getRow(cursor->table->tableId);
    for (int i = nItems; --i >= 0;) {
        if (db->evaluate(condition, item[i], table, cursor)) {
            if (!cursor->add(item[i])) {
                return false;
            }
        }
    }
    if (left != 0) {
        return ((dbTtreeNode*)db->get(left))->traverseBackward(db, cursor,
                                                              condition);
    }
    return true;
}

END_FASTDB_NAMESPACE
