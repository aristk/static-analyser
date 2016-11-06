/******************************************
Copyright (c) 2016, Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#include "propengine.h"
#include <cmath>
#include <string.h>
#include <algorithm>
#include <limits.h>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "solver.h"
#include "clauseallocator.h"
#include "clause.h"
#include "time_mem.h"
#include "varupdatehelper.h"
#include "watchalgos.h"

using namespace CMSat;
using std::cout;
using std::endl;

//#define DEBUG_ENQUEUE_LEVEL0
//#define VERBOSE_DEBUG_POLARITIES
//#define DEBUG_DYNAMIC_RESTART

/**
@brief Sets a sane default config and allocates handler classes
*/
PropEngine::PropEngine(
    const SolverConf* _conf, std::atomic<bool>* _must_interrupt_inter
) :
        CNF(_conf, _must_interrupt_inter)
        , qhead(0)
{
}

PropEngine::~PropEngine()
{
}

void PropEngine::new_var(const bool bva, uint32_t orig_outer)
{
    CNF::new_var(bva, orig_outer);
    //TODO
    //trail... update x->whatever
}

void PropEngine::new_vars(size_t n)
{
    CNF::new_vars(n);
    //TODO
    //trail... update x->whatever
}

void PropEngine::save_on_var_memory()
{
    CNF::save_on_var_memory();
}


void PropEngine::attach_tri_clause(
    Lit lit1
    , Lit lit2
    , Lit lit3
    , const bool red
    , const bool
    #ifdef DEBUG_ATTACH
    checkUnassignedFirst
    #endif
) {
    #ifdef DEBUG_ATTACH
    if (checkUnassignedFirst) {
        assert(lit1.var() != lit2.var());
        assert(value(lit1.var()) == l_Undef);
        assert(value(lit2) == l_Undef || value(lit2) == l_False);

        assert(varData[lit1.var()].removed == Removed::none);
        assert(varData[lit2.var()].removed == Removed::none);
    }
    #endif //DEBUG_ATTACH

    //Order them
    orderLits(lit1, lit2, lit3);

    //And now they are attached, ordered
    watches[lit1].push(Watched(lit2, lit3, red));
    watches[lit2].push(Watched(lit1, lit3, red));
    watches[lit3].push(Watched(lit1, lit2, red));
}

void PropEngine::detach_tri_clause(
    const Lit lit1
    , const Lit lit2
    , const Lit lit3
    , const bool red
    , const bool allow_empty_watch
) {
    Lit lits[3];
    lits[0] = lit1;
    lits[1] = lit2;
    lits[2] = lit3;
    orderLits(lits[0], lits[1], lits[2]);
    if (!(allow_empty_watch && watches[lits[0]].empty())) {
        removeWTri(watches, lits[0], lits[1], lits[2], red);
    }
    if (!(allow_empty_watch && watches[lits[1]].empty())) {
        removeWTri(watches, lits[1], lits[0], lits[2], red);
    }
    if (!(allow_empty_watch && watches[lits[2]].empty())) {
        removeWTri(watches, lits[2], lits[0], lits[1], red);
    }
}

void PropEngine::detach_bin_clause(
    const Lit lit1
    , const Lit lit2
    , const bool red
    , const bool allow_empty_watch
) {
    if (!(allow_empty_watch && watches[lit1].empty())) {
        removeWBin(watches, lit1, lit2, red);
    }
    if (!(allow_empty_watch && watches[lit2].empty())) {
        removeWBin(watches, lit2, lit1, red);
    }
}

void PropEngine::attach_bin_clause(
    const Lit lit1
    , const Lit lit2
    , const bool red
    , const bool
    #ifdef DEBUG_ATTACH
    checkUnassignedFirst
    #endif
) {
    #ifdef DEBUG_ATTACH
    assert(lit1.var() != lit2.var());
    if (checkUnassignedFirst) {
        assert(value(lit1.var()) == l_Undef);
        assert(value(lit2) == l_Undef || value(lit2) == l_False);
    }

    assert(varData[lit1.var()].removed == Removed::none);
    assert(varData[lit2.var()].removed == Removed::none);
    #endif //DEBUG_ATTACH

    watches[lit1].push(Watched(lit2, red));
    watches[lit2].push(Watched(lit1, red));
}

/**
 @ *brief Attach normal a clause to the watchlists

 Handles 2, 3 and >3 clause sizes differently and specially
 */

void PropEngine::attachClause(
    const Clause& c
    , const bool checkAttach
) {
    const ClOffset offset = cl_alloc.get_offset(&c);

    assert(c.size() > 3);
    if (checkAttach) {
        assert(value(c[0]) == l_Undef);
        assert(value(c[1]) == l_Undef || value(c[1]) == l_False);
    }

    #ifdef DEBUG_ATTACH
    for (uint32_t i = 0; i < c.size(); i++) {
        assert(varData[c[i].var()].removed == Removed::none);
    }
    #endif //DEBUG_ATTACH

    const Lit blocked_lit = c[2];
    watches[c[0]].push(Watched(offset, blocked_lit));
    watches[c[1]].push(Watched(offset, blocked_lit));
}

/**
@brief Detaches a (potentially) modified clause

The first two literals might have chaned through modification, so they are
passed along as arguments -- they are needed to find the correct place where
the clause is
*/
void PropEngine::detach_modified_clause(
    const Lit lit1
    , const Lit lit2
    , const uint32_t origSize
    , const Clause* address
) {
    assert(origSize > 3);

    ClOffset offset = cl_alloc.get_offset(address);
    removeWCl(watches[lit1], offset);
    removeWCl(watches[lit2], offset);
}

/**
@brief Propagates a binary clause

Need to be somewhat tricky if the clause indicates that current assignement
is incorrect (i.e. both literals evaluate to FALSE). If conflict if found,
sets failBinLit
*/
template<bool update_bogoprops>
inline bool PropEngine::prop_bin_cl(
    const Watched* i
    , const Lit p
    , PropBy& confl
) {
    const lbool val = value(i->lit2());
    if (val == l_Undef) {
        #ifdef STATS_NEEDED
        if (i->red())
            propStats.propsBinRed++;
        else
            propStats.propsBinIrred++;
        #endif

        enqueue<update_bogoprops>(i->lit2(), PropBy(~p, i->red()));
    } else if (val == l_False) {
        #ifdef STATS_NEEDED
        if (i->red())
            lastConflictCausedBy = ConflCausedBy::binred;
        else
            lastConflictCausedBy = ConflCausedBy::binirred;
        #endif

        confl = PropBy(~p, i->red());
        failBinLit = i->lit2();
        qhead = trail.size();
        return false;
    }

    return true;
}

inline void PropEngine::update_glue(Clause& c)
{
    if (conf.update_glues_on_prop
        && c.red()
        && c.stats.glue > conf.glue_must_keep_clause_if_below_or_eq
    ) {
        const uint32_t new_glue = calc_glue(c);
        if (new_glue < c.stats.glue
            && new_glue < conf.protect_cl_if_improved_glue_below_this_glue_for_one_turn
        ) {
            c.stats.ttl = 1;
        }
        c.stats.glue = std::min(c.stats.glue, new_glue);
    }
}

inline PropResult PropEngine::prop_long_cl_strict_order(
    Watched* i
    , Watched*& j
    , const Lit p
    , PropBy& confl
) {
    //Blocked literal is satisfied, so clause is satisfied
    const Lit blocked = i->getBlockedLit();
    if (value(blocked) == l_True) {
        *j++ = *i;
        return PROP_NOTHING;
    }

    //Dereference pointer
    propStats.bogoProps += 4;
    const ClOffset offset = i->get_offset();
    Clause& c = *cl_alloc.ptr(offset);

    PropResult ret = prop_normal_helper(c, offset, j, p);
    if (ret != PROP_TODO)
        return ret;

    // Did not find watch -- clause is unit under assignment:
    *j++ = *i;
    if (value(c[0]) == l_False) {
        return handle_normal_prop_fail(c, offset, confl);
    }

    //Update stats
    #ifdef STATS_NEEDED
    c.stats.propagations_made++;
    if (c.red())
        propStats.propsLongRed++;
    else
        propStats.propsLongIrred++;
    #endif

    enqueue(c[0], PropBy(offset));

    return PROP_SOMETHING;
}

template<bool update_bogoprops>
inline
bool PropEngine::prop_long_cl_any_order(
    Watched* i
    , Watched*& j
    , const Lit p
    , PropBy& confl
) {
    //Blocked literal is satisfied, so clause is satisfied
    if (value(i->getBlockedLit()) == l_True) {
        *j++ = *i;
        return true;
    }
    if (update_bogoprops) {
        propStats.bogoProps += 4;
    }
    const ClOffset offset = i->get_offset();
    Clause& c = *cl_alloc.ptr(offset);

    #ifdef SLOW_DEBUG
    assert(!c.getRemoved());
    assert(!c.freed());
    #endif

    if (prop_normal_helper(c, offset, j, p) == PROP_NOTHING) {
        return true;
    }

    // Did not find watch -- clause is unit under assignment:
    *j++ = *i;
    if (value(c[0]) == l_False) {
        handle_normal_prop_fail(c, offset, confl);
        return false;
    } else {
        #ifdef STATS_NEEDED
        c.stats.propagations_made++;
        if (c.red())
            propStats.propsLongRed++;
        else
            propStats.propsLongIrred++;
        #endif

        enqueue<update_bogoprops>(c[0], PropBy(offset));
        if (!update_bogoprops) {
            update_glue(c);
        }
    }

    return true;
}

PropResult PropEngine::handle_prop_tri_fail(
    Watched* i
    , Lit lit1
    , PropBy& confl
) {
    #ifdef VERBOSE_DEBUG_FULLPROP
    cout << "Conflict from "
        << lit1 << " , "
        << i->lit2() << " , "
        << i->lit3() << endl;
    #endif //VERBOSE_DEBUG_FULLPROP
    confl = PropBy(~lit1, i->lit3(), i->red());

    #ifdef STATS_NEEDED
    if (i->red())
        lastConflictCausedBy = ConflCausedBy::trired;
    else
        lastConflictCausedBy = ConflCausedBy::triirred;
    #endif

    failBinLit = i->lit2();
    qhead = trail.size();
    return PROP_FAIL;
}

inline PropResult PropEngine::prop_tri_cl_strict_order(
    Watched* i
    , const Lit lit1
    , PropBy& confl
) {
    const Lit lit2 = i->lit2();
    lbool val2 = value(lit2);

    //literal is already satisfied, nothing to do
    if (val2 == l_True)
        return PROP_NOTHING;

    const Lit lit3 = i->lit3();
    lbool val3 = value(lit3);

    //literal is already satisfied, nothing to do
    if (val3 == l_True)
        return PROP_NOTHING;

    if (val2 == l_False && val3 == l_False) {
        return handle_prop_tri_fail(i, lit1, confl);
    }

    if (val2 == l_Undef && val3 == l_False) {
        return propTriHelperSimple(lit1, lit2, lit3, i->red());
    }

    if (val3 == l_Undef && val2 == l_False) {
        return propTriHelperSimple(lit1, lit3, lit2, i->red());
    }

    return PROP_NOTHING;
}

template<bool update_bogoprops>
inline bool PropEngine::prop_tri_cl_any_order(
    Watched* i
    , const Lit lit1
    , PropBy& confl
) {
    const Lit lit2 = i->lit2();
    lbool val2 = value(lit2);

    //literal is already satisfied, nothing to do
    if (val2 == l_True)
        return true;

    const Lit lit3 = i->lit3();
    lbool val3 = value(lit3);

    //literal is already satisfied, nothing to do
    if (val3 == l_True)
        return true;

    if (val2 == l_False && val3 == l_False) {
        #ifdef VERBOSE_DEBUG_FULLPROP
        cout << "Conflict from "
            << lit1 << " , "
            << i->lit2() << " , "
            << i->lit3() << endl;
        #endif //VERBOSE_DEBUG_FULLPROP
        confl = PropBy(~lit1, i->lit3(), i->red());

        #ifdef STATS_NEEDED
        if (i->red())
            lastConflictCausedBy = ConflCausedBy::trired;
        else
            lastConflictCausedBy = ConflCausedBy::triirred;
        #endif

        failBinLit = i->lit2();
        qhead = trail.size();
        return false;
    }
    if (val2 == l_Undef && val3 == l_False) {
        propTriHelperAnyOrder<update_bogoprops>(
            lit1
            , lit2
            , lit3
            , i->red()
        );
        return true;
    }

    if (val3 == l_Undef && val2 == l_False) {
        propTriHelperAnyOrder<update_bogoprops>(
            lit1
            , lit3
            , lit2
            , i->red()
        );
        return true;
    }

    return true;
}

inline PropResult PropEngine::propTriHelperSimple(
    const Lit lit1
    , const Lit lit2
    , const Lit lit3
    , const bool red
) {
    #ifdef STATS_NEEDED
    if (red)
        propStats.propsTriRed++;
    else
        propStats.propsTriIrred++;
    #endif

    enqueue(lit2, PropBy(~lit1, lit3, red));
    return PROP_SOMETHING;
}

template<bool update_bogoprops>
inline void PropEngine::propTriHelperAnyOrder(
    const Lit lit1
    , const Lit lit2
    , const Lit lit3
    , const bool red
) {
    #ifdef STATS_NEEDED
    if (red)
        propStats.propsTriRed++;
    else
        propStats.propsTriIrred++;
    #endif

    //Lazy hyper-bin is not possibe
    enqueue<update_bogoprops>(lit2, PropBy(~lit1, lit3, red));
}

#ifdef __GNUC__
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
__attribute__((optimize("no-unroll-loops")))
#else
#define likely(x)      (x)
#define unlikely(x)    (x)
#endif
PropBy PropEngine::propagate_any_order_fast()
{
    PropBy confl;

    #ifdef VERBOSE_DEBUG_PROP
    cout << "Fast Propagation started" << endl;
    #endif

    int64_t num_props = 0;
    while (qhead < trail.size()) {
        const Lit p = trail[qhead++];     // 'p' is enqueued fact to propagate.
        watch_subarray ws = watches[~p];
        Watched* i;
        Watched* j;
        Watched* end;
        num_props++;

        for (i = j = ws.begin(), end = ws.end(); i != end;) {
            //Prop bin clause
            if (unlikely(i->isBin())) {
                assert(j < end);
                *j++ = *i;
                const lbool val = value(i->lit2());
                if (val == l_Undef) {
                    enqueue<false>(i->lit2(), PropBy(~p, i->red()));
                    i++;
                } else if (val == l_False) {
                    confl = PropBy(~p, i->red());
                    failBinLit = i->lit2();
                    #ifdef STATS_NEEDED
                    if (i->red())
                        lastConflictCausedBy = ConflCausedBy::binred;
                    else
                        lastConflictCausedBy = ConflCausedBy::binirred;
                    #endif
                    i++;
                    while (i < end) {
                        *j++ = *i++;
                    }
                    qhead = trail.size();
                } else {
                    i++;
                }
                continue;
            }

            //Propagate tri clause
            if (unlikely(i->isTri())) {
                *j++ = *i;
                const Lit lit2 = i->lit2();
                lbool val2 = value(lit2);

                //literal is already satisfied, nothing to do
                if (val2 == l_True) {
                    i++;
                    continue;
                }

                const Lit lit3 = i->lit3();
                lbool val3 = value(lit3);

                if (val3 == l_True) {
                    i++;
                    continue;
                }

                if (val2 == l_Undef && val3 == l_False) {
                    enqueue<false>(lit2, PropBy(~p, lit3, i->red()));
                    i++;
                    continue;
                }

                if (val3 == l_Undef && val2 == l_False) {
                    enqueue<false>(lit3, PropBy(~p, lit2, i->red()));
                    i++;
                    continue;
                }

                if (val2 == l_False && val3 == l_False) {
                    confl = PropBy(~p, i->lit3(), i->red());
                    failBinLit = i->lit2();
                    #ifdef STATS_NEEDED
                    if (i->red())
                        lastConflictCausedBy = ConflCausedBy::trired;
                    else
                        lastConflictCausedBy = ConflCausedBy::triirred;
                    #endif
                    i++;
                    while (i < end) {
                        *j++ = *i++;
                    }
                    qhead = trail.size();
                    continue;
                }
                i++;
                continue;
            }

            //propagate normal clause
            //assert(i->isClause());
            Lit blocked = i->getBlockedLit();
            if (value(blocked) == l_True) {
                *j++ = *i++;
                continue;
            }

            const ClOffset offset = i->get_offset();
            Clause& c = *cl_alloc.ptr(offset);
            Lit      false_lit = ~p;
            if (c[0] == false_lit) {
                c[0] = c[1], c[1] = false_lit;
            }
            assert(c[1] == false_lit);
            i++;

            Lit     first = c[0];
            Watched w     = Watched(offset, first);
            if (first != blocked && value(first) == l_True) {
                *j++ = w;
                continue;
            }

            // Look for new watch:
            for (uint32_t k = 2; k < c.size(); k++) {
                //Literal is either unset or satisfied, attach to other watchlist
                if (value(c[k]) != l_False) {
                    c[1] = c[k];
                    c[k] = false_lit;
                    watches[c[1]].push(w);
                    goto nextClause;
                }
            }

            // Did not find watch -- clause is unit under assignment:
            *j++ = w;
            if (value(c[0]) == l_False) {
                confl = PropBy(offset);
                #ifdef STATS_NEEDED
                if (c.red())
                    lastConflictCausedBy = ConflCausedBy::longred;
                else
                    lastConflictCausedBy = ConflCausedBy::longirred;
                #endif
                while (i < end) {
                    *j++ = *i++;
                }
                assert(j <= end);
                qhead = trail.size();
            } else {
                enqueue<false>(c[0], PropBy(offset));
                update_glue(c);
            }

            nextClause:;
        }
        ws.shrink_(i-j);
    }
    qhead = trail.size();
    simpDB_props -= num_props;
    propStats.propagations += (uint64_t)num_props;

    #ifdef VERBOSE_DEBUG
    cout << "Propagation (propagate_any_order) ended." << endl;
    #endif

    return confl;
}

template<bool update_bogoprops>
PropBy PropEngine::propagate_any_order()
{
    PropBy confl;

    #ifdef VERBOSE_DEBUG_PROP
    cout << "Fast Propagation started" << endl;
    #endif

    while (qhead < trail.size() && confl.isNULL()) {
        const Lit p = trail[qhead];     // 'p' is enqueued fact to propagate.
        watch_subarray ws = watches[~p];
        Watched* i = ws.begin();
        Watched* j = i;
        Watched* end = ws.end();
        if (update_bogoprops) {
            propStats.bogoProps += ws.size()/4 + 1;
        }
        propStats.propagations++;
        for (; i != end; i++) {
            if (i->isBin()) {
                *j++ = *i;
                if (!prop_bin_cl<update_bogoprops>(i, p, confl)) {
                    i++;
                    break;
                }
                continue;
            }

            //Propagate tri clause
            if (i->isTri()) {
                *j++ = *i;
                if (!prop_tri_cl_any_order<update_bogoprops>(i, p, confl)) {
                    i++;
                    break;
                }
                continue;
            }

            //propagate normal clause
            if (!prop_long_cl_any_order<update_bogoprops>(i, j, p, confl)) {
                i++;
                break;
            }
            continue;
        }
        while (i != end) {
            *j++ = *i++;
        }
        ws.shrink_(end-j);
        qhead++;
    }

    #ifdef VERBOSE_DEBUG
    cout << "Propagation (propagate_any_order) ended." << endl;
    #endif

    return confl;
}
template PropBy PropEngine::propagate_any_order<true>();
template PropBy PropEngine::propagate_any_order<false>();

void PropEngine::sortWatched()
{
    #ifdef VERBOSE_DEBUG
    cout << "Sorting watchlists:" << endl;
    #endif

    const double myTime = cpuTime();
    for (size_t i = 0
        ; i < watches.watches.size()
        ; ++i
    ) {
        vec<Watched>& ws = watches.watches[i];
        if (ws.size() <= 1)
            continue;

        #ifdef VERBOSE_DEBUG
        cout << "Before sorting: ";
        for (uint32_t i2 = 0; i2 < ws.size(); i2++) {
            if (ws[i2].isBin()) cout << "Binary,";
            if (ws[i2].isTri()) cout << "Tri,";
            if (ws[i2].isClause()) cout << "Normal,";
        }
        cout << endl;
        #endif //VERBOSE_DEBUG

        vec<Watched> sorted;
        for(Watched& w: ws) {
            if (w.isBin()) {
                sorted.push(w);
            }
        }
        for(Watched& w: ws) {
            if (!w.isBin()) {
                sorted.push(w);
            }
        }
        sorted.swap(ws);

        #ifdef VERBOSE_DEBUG
        cout << "After sorting : ";
        for (uint32_t i2 = 0; i2 < ws.size(); i2++) {
            if (ws[i2].isBin()) cout << "Binary,";
            if (ws[i2].isTri()) cout << "Tri,";
            if (ws[i2].isClause()) cout << "Normal,";
        }
        cout << endl;
        cout << " -- " << endl;
        #endif //VERBOSE_DEBUG
    }

    if (conf.verbosity) {
        cout << "c [w-sort] "
        << conf.print_times(cpuTime()-myTime)
        << endl;
    }
}

void PropEngine::printWatchList(const Lit lit) const
{
    watch_subarray_const ws = watches[lit];
    for (const Watched *it2 = ws.begin(), *end2 = ws.end()
        ; it2 != end2
        ; it2++
    ) {
        if (it2->isBin()) {
            cout << "bin: " << lit << " , " << it2->lit2() << " red : " <<  (it2->red()) << endl;
        } else if (it2->isTri()) {
            cout << "tri: " << lit << " , " << it2->lit2() << " , " <<  (it2->lit3()) << endl;
        } else if (it2->isClause()) {
            cout << "cla:" << it2->get_offset() << endl;
        } else {
            assert(false);
        }
    }
}

void PropEngine::updateVars(
    const vector<uint32_t>& outerToInter
    , const vector<uint32_t>& interToOuter
    , const vector<uint32_t>& interToOuter2
) {
    updateArray(varData, interToOuter);
    updateArray(assigns, interToOuter);
    assert(decisionLevel() == 0);

    //Trail is NOT correct, only its length is correct
    for(Lit& lit: trail) {
        lit = lit_Undef;
    }
    updateBySwap(watches, seen, interToOuter2);

    for(watch_subarray w: watches) {
        if (!w.empty())
            updateWatch(w, outerToInter);
    }
}

inline void PropEngine::updateWatch(
    watch_subarray ws
    , const vector<uint32_t>& outerToInter
) {
    for(Watched *it = ws.begin(), *end = ws.end()
        ; it != end
        ; ++it
    ) {
        if (it->isBin()) {
            it->setLit2(
                getUpdatedLit(it->lit2(), outerToInter)
            );

            continue;
        }

        if (it->isTri()) {
            Lit lit1 = it->lit2();
            Lit lit2 = it->lit3();
            lit1 = getUpdatedLit(lit1, outerToInter);
            lit2 = getUpdatedLit(lit2, outerToInter);
            if (lit1 > lit2)
                std::swap(lit1, lit2);

            it->setLit2(lit1);
            it->setLit3(lit2);

            continue;
        }

        if (it->isClause()) {
            it->setBlockedLit(
                getUpdatedLit(it->getBlockedLit(), outerToInter)
            );
        }
    }
}

PropBy PropEngine::propagate_strict_order()
{
    PropBy confl;

    #ifdef VERBOSE_DEBUG_PROP
    cout << "Propagation started" << endl;
    #endif

    uint32_t qheadlong = qhead;

    startAgain:
    //Propagate binary clauses first
    while (qhead < trail.size() && confl.isNULL()) {
        const Lit p = trail[qhead++];     // 'p' is enqueued fact to propagate.
        watch_subarray_const ws = watches[~p];
        const Watched* i = ws.begin();
        const Watched* end = ws.end();
        propStats.bogoProps += ws.size()/10 + 1;
        for (; i != end; i++) {

            //Propagate binary clause
            if (i->isBin()) {
                if (!prop_bin_cl(i, p, confl)) {
                    break;
                }

                continue;
            }

            //Pre-fetch long clause
            if (i->isClause()) {
                if (value(i->getBlockedLit()) != l_True) {
                    const ClOffset offset = i->get_offset();
                    __builtin_prefetch(cl_alloc.ptr(offset));
                }

                continue;
            } //end CLAUSE
        }
    }

    PropResult ret = PROP_NOTHING;
    while (qheadlong < qhead && confl.isNULL()) {
        const Lit p = trail[qheadlong];     // 'p' is enqueued fact to propagate.
        watch_subarray ws = watches[~p];
        Watched* i = ws.begin();
        Watched* j = ws.begin();
        const Watched* end = ws.end();
        propStats.bogoProps += ws.size()/4 + 1;
        for (; i != end; i++) {
            //Skip binary clauses
            if (i->isBin()) {
                *j++ = *i;
                continue;
            }

            if (i->isTri()) {
                *j++ = *i;
                //Propagate tri clause
                ret = prop_tri_cl_strict_order(i, p, confl);
                 if (ret == PROP_SOMETHING || ret == PROP_FAIL) {
                    //Conflict or propagated something
                    i++;
                    break;
                } else {
                    //Didn't propagate anything, continue
                    assert(ret == PROP_NOTHING);
                    continue;
                }
            } //end TRICLAUSE

            if (i->isClause()) {
                ret = prop_long_cl_strict_order(i, j, p, confl);
                 if (ret == PROP_SOMETHING || ret == PROP_FAIL) {
                    //Conflict or propagated something
                    i++;
                    break;
                } else {
                    //Didn't propagate anything, continue
                    assert(ret == PROP_NOTHING);
                    continue;
                }
            } //end CLAUSE
        }
        while (i != end) {
            *j++ = *i++;
        }
        ws.shrink_(end-j);

        //If propagated something, goto start
        if (ret == PROP_SOMETHING) {
            goto startAgain;
        }

        qheadlong++;
    }

    #ifdef VERBOSE_DEBUG
    cout << "Propagation (propagate_strict_order) ended." << endl;
    #endif

    return confl;
}

PropBy PropEngine::propagateIrredBin()
{
    PropBy confl;
    while (qhead < trail.size()) {
        Lit p = trail[qhead++];
        watch_subarray ws = watches[~p];
        for(Watched* k = ws.begin(), *end = ws.end(); k != end; k++) {

            //If not binary, or is redundant, skip
            if (!k->isBin() || k->red())
                continue;

            //Propagate, if conflict, exit
            if (!prop_bin_cl(k, p, confl))
                return confl;
        }
    }

    //No conflict, propagation done
    return PropBy();
}

void PropEngine::print_trail()
{
    for(size_t i = trail_lim[0]; i < trail.size(); i++) {
        cout
        << "trail " << i << ":" << trail[i]
        << " lev: " << varData[trail[i].var()].level
        << " reason: " << varData[trail[i].var()].reason
        << endl;
    }
}


bool PropEngine::propagate_occur()
{
    if (!ok)
        return false;

    assert(decisionLevel() == 0);

    while (qhead < trail_size()) {
        const Lit p = trail[qhead];
        qhead++;
        watch_subarray ws = watches[~p];

        //Go through each occur
        for (const Watched* it = ws.begin(), *end = ws.end()
            ; it != end
            ; ++it
        ) {
            if (it->isClause()) {
                if (!propagate_long_clause_occur(it->get_offset()))
                    return false;
            }

            if (it->isTri()) {
                if (!propagate_tri_clause_occur(*it))
                    return false;
            }

            if (it->isBin()) {
                if (!propagate_binary_clause_occur(*it))
                    return false;
            }
        }
    }

    return true;
}

bool PropEngine::propagate_tri_clause_occur(const Watched& ws)
{
    const lbool val2 = value(ws.lit2());
    const lbool val3 = value(ws.lit3());
    if (val2 == l_True
        || val3 == l_True
    ) {
        return true;
    }

    if (val2 == l_Undef
        && val3 == l_Undef
    ) {
        return true;
    }

    if (val2 == l_False
        && val3 == l_False
    ) {
        ok = false;
        return false;
    }

    #ifdef STATS_NEEDED
    if (ws.red())
        propStats.propsTriRed++;
    else
        propStats.propsTriIrred++;
    #endif

    if (val2 == l_Undef) {
        enqueue(ws.lit2());
    } else {
        enqueue(ws.lit3());
    }
    return true;
}

bool PropEngine::propagate_binary_clause_occur(const Watched& ws)
{
    const lbool val = value(ws.lit2());
    if (val == l_False) {
        ok = false;
        return false;
    }

    if (val == l_Undef) {
        enqueue(ws.lit2());
        #ifdef STATS_NEEDED
        if (ws.red())
            propStats.propsBinRed++;
        else
            propStats.propsBinIrred++;
        #endif
    }

    return true;
}

bool PropEngine::propagate_long_clause_occur(const ClOffset offset)
{
    const Clause& cl = *cl_alloc.ptr(offset);
    assert(!cl.freed() && "Cannot be already removed in occur");

    Lit lastUndef = lit_Undef;
    uint32_t numUndef = 0;
    bool satisfied = false;
    for (const Lit lit: cl) {
        const lbool val = value(lit);
        if (val == l_True) {
            satisfied = true;
            break;
        }
        if (val == l_Undef) {
            numUndef++;
            if (numUndef > 1) break;
            lastUndef = lit;
        }
    }
    if (satisfied)
        return true;

    //Problem is UNSAT
    if (numUndef == 0) {
        ok = false;
        return false;
    }

    if (numUndef > 1)
        return true;

    enqueue(lastUndef);
    #ifdef STATS_NEEDED
    if (cl.size() == 3)
        if (cl.red())
            propStats.propsTriRed++;
        else
            propStats.propsTriIrred++;
    else {
        if (cl.red())
            propStats.propsLongRed++;
        else
            propStats.propsLongIrred++;
    }
    #endif

    return true;
}

void PropEngine::save_state(SimpleOutFile& f) const
{
    f.put_vector(trail);
    f.put_uint32_t(qhead);

    CNF::save_state(f);
}

void PropEngine::load_state(SimpleInFile& f)
{
    f.get_vector(trail);
    qhead = f.get_uint32_t();

    CNF::load_state(f);
}
