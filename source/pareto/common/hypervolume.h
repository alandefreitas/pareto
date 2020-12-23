//
// Created by Alan Freitas on 12/18/20.
//

#ifndef PARETO_HYPERVOLUME_H
#define PARETO_HYPERVOLUME_H

/// Hypervolume computation in C++
/// Adapted from:
/// http://lopez-ibanez.eu/hypervolume
/// We reimplemented their algorithm because:
/// 1) We needed to make this thread-safe
/// 2) It doesn't pass all modern C++ checks
/// 3) It should handle other types with C++ templates
/// 4) The code becomes much easier to read with C++
/// 5) This was the only code in this library that was not header-only
///
/// We could still make this header have only a few lines of code
/// just by using std::set (less 313+ lines of code) and std::list
/// (less 144+ lines of code) instead. This would also allow our
/// containers to work to calculate hypervolumes
/// directly in other data types (to be implemented).

#include <cassert>
#include <cstdlib>
#include <limits>
#include <variant>

namespace pareto {
    namespace detail {
        constexpr int stop_dimension = 2; /* default: stop on dimension 3 */

        inline int compare_tree_asc(const void *p1, const void *p2);

        /* User supplied function to compare two items like strcmp() does.
         * For example: cmp(a,b) will return:
         *   -1  if a < b
         *    0  if a = b
         *    1  if a > b
         */
        typedef int (*avl_compare_t)(const void *, const void *);

        /* User supplied function to delete an item when a node is free()d.
         * If nullptr, the item is not free()d.
         */
        typedef void (*avl_freeitem_t)(void *);

        typedef struct avl_node_t {
            struct avl_node_t *next;
            struct avl_node_t *prev;
            struct avl_node_t *parent;
            struct avl_node_t *left;
            struct avl_node_t *right;
            void *item;
            double domr;
            unsigned char depth;
        } avl_node_t;

        typedef struct avl_tree_t {
            avl_node_t *head;
            avl_node_t *top;
        } avl_tree_t;

        inline void avl_rebalance(avl_tree_t *, avl_node_t *);

        constexpr int NODE_DEPTH(avl_node_t *n) {
            return ((n) ? (n)->depth : 0);
        }
        constexpr int L_DEPTH(avl_node_t *n) { return (NODE_DEPTH((n)->left)); }
        constexpr int R_DEPTH(avl_node_t *n) {
            return (NODE_DEPTH((n)->right));
        }
        constexpr int CALC_DEPTH(avl_node_t *n) {
            return ((L_DEPTH(n) > R_DEPTH(n) ? L_DEPTH(n) : R_DEPTH(n)) + 1);
        }

        inline int avl_check_balance(avl_node_t *avlnode) {
            int d;
            d = R_DEPTH(avlnode) - L_DEPTH(avlnode);
            return d < -1 ? -1 : d > 1 ? 1 : 0;
        }

        inline int avl_search_closest(const avl_tree_t *avltree,
                                      const void *item, avl_node_t **avlnode) {
            avl_node_t *node;
            int c;

            if (!avlnode)
                avlnode = &node;

            node = avltree->top;

            if (!node)
                return *avlnode = nullptr, 0;

            for (;;) {
                c = compare_tree_asc(item, node->item);

                if (c < 0) {
                    if (node->left)
                        node = node->left;
                    else
                        return *avlnode = node, -1;
                } else if (c > 0) {
                    if (node->right)
                        node = node->right;
                    else
                        return *avlnode = node, 1;
                } else {
                    return *avlnode = node, 0;
                }
            }
        }

        inline avl_tree_t *avl_init_tree(avl_tree_t *rc, avl_compare_t,
                                         avl_freeitem_t) {
            if (rc) {
                rc->head = nullptr;
                rc->top = nullptr;
            }
            return rc;
        }

        inline avl_tree_t *avl_alloc_tree(avl_compare_t cmp,
                                          avl_freeitem_t freeitem) {
            return avl_init_tree(new avl_tree_t, cmp, freeitem);
        }

        inline void avl_clear_tree(avl_tree_t *avltree) {
            avltree->top = avltree->head = nullptr;
        }

        inline void avl_clear_node(avl_node_t *newnode) {
            newnode->left = newnode->right = nullptr;
            newnode->depth = 1;
        }

        inline avl_node_t *avl_insert_top(avl_tree_t *avltree,
                                          avl_node_t *newnode) {
            avl_clear_node(newnode);
            newnode->prev = newnode->next = newnode->parent = nullptr;
            avltree->head = avltree->top = newnode;
            return newnode;
        }

        inline avl_node_t *avl_insert_before(avl_tree_t *avltree,
                                             avl_node_t *node,
                                             avl_node_t *newnode) {
            assert(node);
            assert(!node->left);

            avl_clear_node(newnode);

            newnode->next = node;
            newnode->parent = node;

            newnode->prev = node->prev;
            if (node->prev)
                node->prev->next = newnode;
            else
                avltree->head = newnode;
            node->prev = newnode;

            node->left = newnode;
            avl_rebalance(avltree, node);
            return newnode;
        }

        inline avl_node_t *avl_insert_after(avl_tree_t *avltree,
                                            avl_node_t *node,
                                            avl_node_t *newnode) {
            assert(node);
            assert(!node->right);

            avl_clear_node(newnode);

            newnode->prev = node;
            newnode->parent = node;

            newnode->next = node->next;
            if (node->next) {
                node->next->prev = newnode;
            }
            node->next = newnode;

            node->right = newnode;
            avl_rebalance(avltree, node);
            return newnode;
        }

        /*
         * avl_unlink_node:
         * Removes the given node.  Does not delete the item at that node.
         * The item of the node may be freed before calling avl_unlink_node.
         * (In other words, it is not referenced by this function.)
         */
        inline void avl_unlink_node(avl_tree_t *avltree, avl_node_t *avlnode) {
            avl_node_t *parent;
            avl_node_t **superparent;
            avl_node_t *subst, *left, *right;
            avl_node_t *balnode;

            if (avlnode->prev)
                avlnode->prev->next = avlnode->next;
            else
                avltree->head = avlnode->next;

            if (avlnode->next) {
                avlnode->next->prev = avlnode->prev;
            }

            parent = avlnode->parent;

            superparent = parent ? avlnode == parent->left ? &parent->left
                                                           : &parent->right
                                 : &avltree->top;

            left = avlnode->left;
            right = avlnode->right;
            if (!left) {
                *superparent = right;
                if (right)
                    right->parent = parent;
                balnode = parent;
            } else if (!right) {
                *superparent = left;
                left->parent = parent;
                balnode = parent;
            } else {
                subst = avlnode->prev;
                if (subst == left) {
                    balnode = subst;
                } else {
                    balnode = subst->parent;
                    balnode->right = subst->left;
                    if (balnode->right)
                        balnode->right->parent = balnode;
                    subst->left = left;
                    left->parent = subst;
                }
                subst->right = right;
                subst->parent = parent;
                right->parent = subst;
                *superparent = subst;
            }

            avl_rebalance(avltree, balnode);
        }

        /*
         * avl_rebalance:
         * Rebalances the containers if one side becomes too heavy.  This
         * function assumes that both subtrees are AVL-trees with consistant
         * data.  The function has the additional side effect of recalculating
         * the count of the containers at this node.  It should be noted that at
         * the return of this function, if a rebalance takes place, the top of
         * this subtree is no longer going to be the same node.
         */
        inline void avl_rebalance(avl_tree_t *avltree, avl_node_t *avlnode) {
            avl_node_t *child;
            avl_node_t *gchild;
            avl_node_t *parent;
            avl_node_t **superparent;

            parent = avlnode;

            while (avlnode) {
                parent = avlnode->parent;

                superparent = parent ? avlnode == parent->left ? &parent->left
                                                               : &parent->right
                                     : &avltree->top;

                switch (avl_check_balance(avlnode)) {
                case -1:
                    child = avlnode->left;
                    if (L_DEPTH(child) >= R_DEPTH(child)) {
                        avlnode->left = child->right;
                        if (avlnode->left)
                            avlnode->left->parent = avlnode;
                        child->right = avlnode;
                        avlnode->parent = child;
                        *superparent = child;
                        child->parent = parent;
                        avlnode->depth = CALC_DEPTH(avlnode);
                        child->depth = CALC_DEPTH(child);
                    } else {
                        gchild = child->right;
                        avlnode->left = gchild->right;
                        if (avlnode->left)
                            avlnode->left->parent = avlnode;
                        child->right = gchild->left;
                        if (child->right)
                            child->right->parent = child;
                        gchild->right = avlnode;
                        if (gchild->right)
                            gchild->right->parent = gchild;
                        gchild->left = child;
                        if (gchild->left)
                            gchild->left->parent = gchild;
                        *superparent = gchild;
                        gchild->parent = parent;
                        avlnode->depth = CALC_DEPTH(avlnode);
                        child->depth = CALC_DEPTH(child);
                        gchild->depth = CALC_DEPTH(gchild);
                    }
                    break;
                case 1:
                    child = avlnode->right;
                    if (R_DEPTH(child) >= L_DEPTH(child)) {
                        avlnode->right = child->left;
                        if (avlnode->right)
                            avlnode->right->parent = avlnode;
                        child->left = avlnode;
                        avlnode->parent = child;
                        *superparent = child;
                        child->parent = parent;
                        avlnode->depth = CALC_DEPTH(avlnode);
                        child->depth = CALC_DEPTH(child);
                    } else {
                        gchild = child->left;
                        avlnode->right = gchild->left;
                        if (avlnode->right)
                            avlnode->right->parent = avlnode;
                        child->left = gchild->right;
                        if (child->left)
                            child->left->parent = child;
                        gchild->left = avlnode;
                        if (gchild->left)
                            gchild->left->parent = gchild;
                        gchild->right = child;
                        if (gchild->right)
                            gchild->right->parent = gchild;
                        *superparent = gchild;
                        gchild->parent = parent;
                        avlnode->depth = CALC_DEPTH(avlnode);
                        child->depth = CALC_DEPTH(child);
                        gchild->depth = CALC_DEPTH(gchild);
                    }
                    break;
                default:
                    avlnode->depth = CALC_DEPTH(avlnode);
                }
                avlnode = parent;
            }
        }

        /*------------------------------------------------------------------------------
         end of functions from AVL-containers library.
        *******************************************************************************/
        typedef struct dlnode {
            double *x;            /* The data vector              */
            struct dlnode **next; /* Next-node vector             */
            struct dlnode **prev; /* Previous-node vector         */
            struct avl_node_t *tnode;
            int ignore;
            int ignore_best; // used in define_order
            double *area;    /* Area */
            double *vol;     /* Volume */
        } dlnode_t;

        inline int compare_node(const void *p1, const void *p2) {
            const double x1 = *((*(const dlnode_t **)p1)->x);
            const double x2 = *((*(const dlnode_t **)p2)->x);

            return (x1 < x2) ? -1 : (x1 > x2) ? 1 : 0;
        }

        inline int compare_tree_asc(const void *p1, const void *p2) {
            const auto *x1 = (const double *)p1;
            const auto *x2 = (const double *)p2;

            return (x1[1] > x2[1])    ? -1
                   : (x1[1] < x2[1])  ? 1
                   : (x1[0] >= x2[0]) ? -1
                                      : 1;
        }

        /*
         * Setup circular double-linked list in each dimension
         */
        inline dlnode_t *setup_cdllist(double *data, int d, int n) {
            dlnode_t *head;
            dlnode_t **scratch;
            int i, j;

            head = reinterpret_cast<dlnode_t *>(
                malloc((n + 1) * sizeof(dlnode_t)));

            head->x = data;
            head->ignore = 0; /* should never get used */
            head->next = reinterpret_cast<dlnode_t **>(
                malloc(d * (n + 1) * sizeof(dlnode_t *)));
            head->prev = reinterpret_cast<dlnode_t **>(
                malloc(d * (n + 1) * sizeof(dlnode_t *)));
            head->tnode = reinterpret_cast<avl_node_t *>(
                malloc((n + 1) * sizeof(avl_node_t)));
            head->area = reinterpret_cast<double *>(
                malloc(d * (n + 1) * sizeof(double)));
            head->vol = reinterpret_cast<double *>(
                malloc(d * (n + 1) * sizeof(double)));

            for (i = 1; i <= n; i++) {
                head[i].x = head[i - 1].x +
                            d; /* this will be fixed a few lines below... */
                head[i].ignore = 0;
                head[i].next = head[i - 1].next + d;
                head[i].prev = head[i - 1].prev + d;
                head[i].tnode = head[i - 1].tnode + 1;
                head[i].area = head[i - 1].area + d;
                head[i].vol = head[i - 1].vol + d;
            }
            head->x = nullptr; /* head contains no data */

            scratch =
                reinterpret_cast<dlnode_t **>(malloc(n * sizeof(dlnode_t *)));
            for (i = 0; i < n; i++)
                scratch[i] = head + i + 1;

            for (j = d - 1; j >= 0; j--) {
                for (i = 0; i < n; i++)
                    scratch[i]->x--;
                qsort(scratch, n, sizeof(dlnode_t *), compare_node);
                head->next[j] = scratch[0];
                scratch[0]->prev[j] = head;
                for (i = 1; i < n; i++) {
                    scratch[i - 1]->next[j] = scratch[i];
                    scratch[i]->prev[j] = scratch[i - 1];
                }
                scratch[n - 1]->next[j] = head;
                head->prev[j] = scratch[n - 1];
            }

            free(scratch);

            for (i = 1; i <= n; i++) {
                (head[i].tnode)->item = head[i].x;
            }

            return head;
        }

        inline void free_cdllist(dlnode_t *head) {
            free(head->tnode); /* Frees _all_ nodes. */
            free(head->next);
            free(head->prev);
            free(head->area);
            free(head->vol);
            free(head);
        }

        inline void delete_dlnode(dlnode_t *nodep, int dim,
                                  std::vector<double> &bound) {
            int i;

            for (i = stop_dimension; i < dim; i++) {
                nodep->prev[i]->next[i] = nodep->next[i];
                nodep->next[i]->prev[i] = nodep->prev[i];
                if (bound[i] > nodep->x[i])
                    bound[i] = nodep->x[i];
            }
        }

        inline void delete_dom(dlnode_t *nodep, int dim) {
            int i;

            for (i = stop_dimension; i < dim; i++) {
                nodep->prev[i]->next[i] = nodep->next[i];
                nodep->next[i]->prev[i] = nodep->prev[i];
            }
        }

        inline void reinsert(dlnode_t *nodep, int dim,
                             std::vector<double> &bound) {
            int i;

            for (i = stop_dimension; i < dim; i++) {
                nodep->prev[i]->next[i] = nodep;
                nodep->next[i]->prev[i] = nodep;
                if (bound[i] > nodep->x[i])
                    bound[i] = nodep->x[i];
            }
        }

        inline void reinsert_dom(dlnode_t *nodep, int dim) {
            int i;
            for (i = stop_dimension; i < dim; i++) {
                dlnode_t *p = nodep->prev[i];
                p->next[i] = nodep;
                nodep->next[i]->prev[i] = nodep;
                nodep->area[i] = p->area[i];
                nodep->vol[i] =
                    p->vol[i] + p->area[i] * (nodep->x[i] - p->x[i]);
            }
        }

        // NOLINTNEXTLINE(misc-no-recursion)
        inline double hv_recursive(avl_tree_t *tree, dlnode_t *list, int dim,
                                   int c, const double *ref,
                                   std::vector<double> &bound) {
            /* ------------------------------------------------------
               General case for dimensions higher than stop_dimension
               ------------------------------------------------------ */
            if (dim > stop_dimension) {
                dlnode_t *p0 = list;
                dlnode_t *p1 = list->prev[dim];
                double hyperv = 0;
                dlnode_t *pp;
                for (pp = p1; pp->x; pp = pp->prev[dim]) {
                    if (pp->ignore < dim)
                        pp->ignore = 0;
                }
                while (c > 1
                       /* We delete all points x[dim] > bound[dim]. In case of
                             repeated coordinates, we also delete all points
                             x[dim] == bound[dim] except one. */
                       && (p1->x[dim] > bound[dim] ||
                           p1->prev[dim]->x[dim] >= bound[dim])) {
                    p0 = p1;
                    if (p0->ignore >= dim)
                        delete_dom(p0, dim);
                    else
                        delete_dlnode(p0, dim, bound);
                    p1 = p0->prev[dim];
                    c--;
                }

                if (c > 1) {
                    hyperv = p1->prev[dim]->vol[dim] +
                             p1->prev[dim]->area[dim] *
                                 (p1->x[dim] - p1->prev[dim]->x[dim]);

                    if (p1->ignore >= dim)
                        p1->area[dim] = p1->prev[dim]->area[dim];
                    else {
                        p1->area[dim] =
                            hv_recursive(tree, list, dim - 1, c, ref, bound);
                        /* At this point, p1 is the point with the highest value
                           in dimension dim in the list, so if it is dominated
                           in dimension dim-1, so it is also dominated in
                           dimension dim. */
                        if (p1->ignore == (dim - 1))
                            p1->ignore = dim;
                    }
                } else {
                    int i;
                    p1->area[0] = 1;
                    for (i = 1; i <= dim; i++)
                        p1->area[i] =
                            p1->area[i - 1] * (ref[i - 1] - p1->x[i - 1]);
                }
                p1->vol[dim] = hyperv;

                while (p0->x != nullptr) {
                    hyperv += p1->area[dim] * (p0->x[dim] - p1->x[dim]);
                    c++;
                    if (p0->ignore >= dim) {
                        reinsert_dom(p0, dim);
                        p0->area[dim] = p1->area[dim];
                    } else {
                        reinsert(p0, dim, bound);
                        p0->area[dim] =
                            hv_recursive(tree, list, dim - 1, c, ref, bound);
                        if (p0->ignore == (dim - 1))
                            p0->ignore = dim;
                    }
                    p1 = p0;
                    p0 = p0->next[dim];
                    p1->vol[dim] = hyperv;
                }
                bound[dim] = p1->x[dim];
                hyperv += p1->area[dim] * (ref[dim] - p1->x[dim]);
                return hyperv;
            }

            /* ---------------------------
               special case of dimension 3
               --------------------------- */
            else if (dim == 2) {
                double hyperv;
                double hypera;
                double height;

                dlnode_t *pp = list->prev[2];
                avl_node_t *tnode;

                /* All the points that have value of x[2] lower than bound[2]
                   are points that were previously processed, so there's no need
                   to process them again.  In this case, every point was
                   processed before, so the volume is known.  */
                if (pp->x[2] < bound[2])
                    return pp->vol[2] + pp->area[2] * (ref[2] - pp->x[2]);

                pp = list->next[2];

                /* In this case, every point has to be processed.  */
                if (pp->x[2] >= bound[2]) {
                    pp->tnode->domr = ref[2];
                    pp->area[2] = (ref[0] - pp->x[0]) * (ref[1] - pp->x[1]);
                    pp->vol[2] = 0;
                    pp->ignore = 0;
                } else {
                    /* Otherwise, we look for the first point that has to be in
                       the containers, by searching for the first point that
                       isn't dominated or that is dominated by a point with
                       value of x[2] higher or equal than bound[2] (domr keeps
                       the value of the x[2] of the point that dominates pp, or
                       ref[2] if it isn't dominated).  */
                    while (pp->tnode->domr < bound[2]) {
                        pp = pp->next[2];
                    }
                }

                pp->ignore = 0;
                avl_insert_top(tree, pp->tnode);
                pp->tnode->domr = ref[2];

                /* Connect all points that aren't dominated or that are
                   dominated and the point that dominates it has value x[2]
                   (pp->tnode->domr) equal or higher than bound[2].  */
                for (pp = pp->next[2]; pp->x[2] < bound[2]; pp = pp->next[2]) {
                    if (pp->tnode->domr >= bound[2]) {
                        avl_node_t *tnodeaux = pp->tnode;
                        tnodeaux->domr = ref[2];
                        if (avl_search_closest(tree, pp->x, &tnode) <= 0)
                            avl_insert_before(tree, tnode, tnodeaux);
                        else
                            avl_insert_after(tree, tnode, tnodeaux);
                    }
                }
                pp = pp->prev[2];
                hyperv = pp->vol[2];
                hypera = pp->area[2];

                height = (pp->next[2]->x) ? pp->next[2]->x[2] - pp->x[2]
                                          : ref[2] - pp->x[2];

                bound[2] = list->prev[2]->x[2];
                hyperv += hypera * height;
                for (pp = pp->next[2]; pp->x != nullptr; pp = pp->next[2]) {
                    const double *prv_ip, *nxt_ip;
                    avl_node_t *tnode_2;
                    int cmp;
                    pp->vol[2] = hyperv;
                    height = (pp == list->prev[2])
                                 ? ref[2] - pp->x[2]
                                 : pp->next[2]->x[2] - pp->x[2];
                    if (pp->ignore >= 2) {
                        hyperv += hypera * height;
                        pp->area[2] = hypera;
                        continue;
                    }
                    cmp = avl_search_closest(tree, pp->x, &tnode_2);
                    if (cmp <= 0) {
                        nxt_ip = (double *)(tnode_2->item);
                    } else {
                        nxt_ip = (tnode_2->next != nullptr)
                                     ? (double *)(tnode_2->next->item)
                                     : ref;
                    }
                    if (nxt_ip[0] <= pp->x[0]) {
                        pp->ignore = 2;
                        pp->tnode->domr = pp->x[2];
                        pp->area[2] = hypera;
                        if (height > 0)
                            hyperv += hypera * height;
                        continue;
                    }
                    if (cmp <= 0) {
                        avl_insert_before(tree, tnode_2, pp->tnode);
                        tnode_2 = pp->tnode->prev;
                    } else {
                        avl_insert_after(tree, tnode_2, pp->tnode);
                    }
                    pp->tnode->domr = ref[2];
                    if (tnode_2 != nullptr) {
                        prv_ip = (double *)(tnode_2->item);
                        if (prv_ip[0] >= pp->x[0]) {
                            const double *cur_ip;

                            tnode_2 = pp->tnode->prev;
                            /* cur_ip = point dominated by pp with highest
                               [0]-coordinate.  */
                            cur_ip = (double *)(tnode_2->item);
                            while (tnode_2->prev) {
                                prv_ip = (double *)(tnode_2->prev->item);
                                hypera -= (prv_ip[1] - cur_ip[1]) *
                                          (nxt_ip[0] - cur_ip[0]);
                                if (prv_ip[0] < pp->x[0])
                                    break; /* prv is not dominated by pp */
                                cur_ip = prv_ip;
                                avl_unlink_node(tree, tnode_2);
                                /* saves the value of x[2] of the point that
                                   dominates tnode_2. */
                                tnode_2->domr = pp->x[2];
                                tnode_2 = tnode_2->prev;
                            }

                            avl_unlink_node(tree, tnode_2);
                            tnode_2->domr = pp->x[2];
                            if (!tnode_2->prev) {
                                hypera -= (ref[1] - cur_ip[1]) *
                                          (nxt_ip[0] - cur_ip[0]);
                                prv_ip = ref;
                            }
                        }
                    } else
                        prv_ip = ref;

                    hypera += (prv_ip[1] - pp->x[1]) * (nxt_ip[0] - pp->x[0]);

                    if (height > 0)
                        hyperv += hypera * height;
                    pp->area[2] = hypera;
                }
                avl_clear_tree(tree);
                return hyperv;
            }

            /* special case of dimension 2 */
            else if (dim == 1) {
                const dlnode_t *p1 = list->next[1];
                double hypera = p1->x[0];
                double hyperv = 0;
                dlnode_t *p0;

                while ((p0 = p1->next[1])->x) {
                    hyperv += (ref[0] - hypera) * (p0->x[1] - p1->x[1]);
                    if (p0->x[0] < hypera)
                        hypera = p0->x[0];
                    else if (p0->ignore == 0)
                        p0->ignore = 1;
                    p1 = p0;
                }
                hyperv += (ref[0] - hypera) * (ref[1] - p1->x[1]);
                return hyperv;
            }

            /* special case of dimension 1 */
            else if (dim == 0) {
                list->next[0]->ignore = -1;
                return (ref[0] - list->next[0]->x[0]);
            } else {
                throw std::logic_error("hypervolume: unreachable condition! \n"
                                       "This is a bug, please report it to "
                                       "manuel.lopez-ibanez@ulb.ac.be\n");
            }
        }

        /*
          Removes the point from the circular double-linked list, but it
          doesn't remove the data.
        */
        inline void filter_delete_node(dlnode_t *node, int d) {
            int i;

            for (i = 0; i < d; i++) {
                node->next[i]->prev[i] = node->prev[i];
                node->prev[i]->next[i] = node->next[i];
            }
        }

        /*
          Filters those points that do not strictly dominate the reference
          point.  This is needed to assure that the points left are only those
          that are needed to calculate the hypervolume.
        */
        inline int filter(dlnode_t *list, int d, int n, const double *ref) {
            int i, j;

            /* fprintf (stderr, "%d points initially\n", n); */
            for (i = 0; i < d; i++) {
                dlnode_t *aux = list->prev[i];
                int np = n;
                for (j = 0; j < np; j++) {
                    if (aux->x[i] < ref[i])
                        break;
                    filter_delete_node(aux, d);
                    aux = aux->prev[i];
                    n--;
                }
            }
            /* fprintf (stderr, "%d points remain\n", n); */
            return n;
        }
    } // namespace detail

    inline double fpli_hv(double *data, int d, int n, const double *ref) {
        // replace with list<struct>
        detail::dlnode_t *list;

        double hyperv;

        std::vector<double> bound(d, -std::numeric_limits<double>::max());
        int i;

        // replace with set
        detail::avl_tree_t *tree;

        tree = detail::avl_alloc_tree(
            (detail::avl_compare_t)detail::compare_tree_asc,
            (detail::avl_freeitem_t) nullptr);

        list = detail::setup_cdllist(data, d, n);

        n = filter(list, d, n, ref);
        if (n == 0) {
            hyperv = 0.0;
        } else if (n == 1) {
            detail::dlnode_t *p = list->next[0];
            hyperv = 1;
            for (i = 0; i < d; i++)
                hyperv *= ref[i] - p->x[i];
        } else {
            hyperv = hv_recursive(tree, list, d - 1, n, ref, bound);
        }

        /* Clean up. */
        free_cdllist(list);
        free(tree); /* The nodes are freed by free_cdllist ().  */

        return hyperv;
    }

} // namespace pareto

#endif // PARETO_HYPERVOLUME_H
