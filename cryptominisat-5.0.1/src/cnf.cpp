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

#include "cnf.h"

#include <stdexcept>

#include "vardata.h"
#include "solvertypes.h"
#include "clauseallocator.h"
#include "watchalgos.h"
#include "varupdatehelper.h"
#include "time_mem.h"

using namespace CMSat;

void CNF::new_var(const bool bva, const uint32_t orig_outer)
{
    if (nVars() >= 1ULL<<28) {
        cout << "ERROR! Variable requested is far too large" << endl;
        throw std::runtime_error("ERROR! Variable requested is far too large");
    }

    minNumVars++;
    enlarge_minimal_datastructs();
    if (conf.doCache) {
        implCache.new_var();
    }
    if (conf.doStamp) {
        stamp.new_var();
    }

    if (orig_outer == std::numeric_limits<uint32_t>::max()) {
        //completely new var
        enlarge_nonminimial_datastructs();

        uint32_t minVar = nVars()-1;
        uint32_t maxVar = nVarsOuter()-1;
        interToOuterMain.push_back(maxVar);
        const uint32_t x = interToOuterMain[minVar];
        interToOuterMain[minVar] = maxVar;
        interToOuterMain[maxVar] = x;

        outerToInterMain.push_back(maxVar);
        outerToInterMain[maxVar] = minVar;
        outerToInterMain[x] = maxVar;

        swapVars(nVarsOuter()-1);
        varData[nVars()-1].is_bva = bva;
        if (bva) {
            num_bva_vars ++;
        } else {
            outer_to_with_bva_map.push_back(nVarsOuter() - 1);
        }
    } else {
        //Old var, re-inserted
        assert(orig_outer < nVarsOuter());

        const uint32_t minVar = nVars()-1;
        uint32_t k = interToOuterMain[minVar];
        uint32_t z = outerToInterMain[orig_outer];
        interToOuterMain[minVar] = orig_outer;
        interToOuterMain[z] = k;

        outerToInterMain[k] = z;
        outerToInterMain[orig_outer] = minVar;

        swapVars(z);
    }

    #ifdef SLOW_DEBUG
    test_reflectivity_of_renumbering();
    #endif
}

void CNF::new_vars(const size_t n)
{
    if (nVars() + n >= 1ULL<<28) {
        cout << "ERROR! Variable requested is far too large" << endl;
        std::exit(-1);
    }

    if (conf.doCache) {
        implCache.new_vars(n);
    }
    if (conf.doStamp) {
        stamp.new_vars(n);
    }

    minNumVars += n;
    enlarge_minimal_datastructs(n);
    enlarge_nonminimial_datastructs(n);

    uint64_t f = interToOuterMain.size() + n;
    f += 0xfff - (f&0xfff);
    interToOuterMain.reserve(f);
    outerToInterMain.reserve(f);
    uint64_t g = outer_to_with_bva_map.size() + n;
    g += 0xfff - (g&0xfff);
    outer_to_with_bva_map.reserve(g);
    for(int i = n-1; i >= 0; i--) {
        const uint32_t minVar = nVars()-i-1;
        const uint32_t maxVar = nVarsOuter()-i-1;

        interToOuterMain.push_back(maxVar);
        const uint32_t x = interToOuterMain[minVar];
        interToOuterMain[minVar] = maxVar;
        interToOuterMain[maxVar] = x;

        outerToInterMain.push_back(maxVar);
        outerToInterMain[maxVar] = minVar;
        outerToInterMain[x] = maxVar;

        swapVars(nVarsOuter()-i-1, i);
        varData[nVars()-i-1].is_bva = false;
        outer_to_with_bva_map.push_back(nVarsOuter()-i-1);
    }

    #ifdef SLOW_DEBUG
    test_reflectivity_of_renumbering();
    #endif
}

void CNF::swapVars(const uint32_t which, const int off_by)
{
    std::swap(assigns[nVars()-off_by-1], assigns[which]);
    std::swap(varData[nVars()-off_by-1], varData[which]);
}

void CNF::enlarge_nonminimial_datastructs(size_t n)
{
    assigns.resize(assigns.size() + n, l_Undef);
    varData.resize(varData.size() + n, VarData());
    depth.resize(depth.size() + n);
}

void CNF::enlarge_minimal_datastructs(size_t n)
{
    watches.resize(watches.size() + 2*n);
    seen.resize(seen.size() + 2*n, 0);
    seen2.resize(seen2.size() + 2*n,0);
    permDiff.resize(seen2.size() + 2*n,0);
}

void CNF::save_on_var_memory()
{
    //never resize varData --> contains info about what is replaced/etc.
    //never resize assigns --> contains 0-level assigns
    //never resize interToOuterMain, outerToInterMain

    watches.resize(nVars()*2);
    watches.consolidate(); //not using the one with SQL because it's already saved
    implCache.save_on_var_memorys(nVars());
    stamp.save_on_var_memory(nVars());
    longRedCls.shrink_to_fit();
    longIrredCls.shrink_to_fit();

    seen.resize(nVars()*2);
    seen.shrink_to_fit();
    seen2.resize(nVars()*2);
    seen2.shrink_to_fit();
    permDiff.resize(nVars()*2);
    permDiff.shrink_to_fit();
}

//Test for reflectivity of interToOuterMain & outerToInterMain
void CNF::test_reflectivity_of_renumbering() const
{
    vector<uint32_t> test(nVarsOuter());
    for(size_t i = 0; i  < nVarsOuter(); i++) {
        test[i] = i;
    }
    updateArrayRev(test, interToOuterMain);
    #ifdef DEBUG_RENUMBER
    for(size_t i = 0; i < nVarsOuter(); i++) {
        cout << i << ": "
        << std::setw(2) << test[i] << ", "
        << std::setw(2) << outerToInterMain[i]
        << endl;
    }
    #endif

    for(size_t i = 0; i < nVarsOuter(); i++) {
        assert(test[i] == outerToInterMain[i]);
    }
    #ifdef DEBUG_RENUMBR
    cout << "Passed test" << endl;
    #endif
}

void CNF::updateVars(
    const vector<uint32_t>& outerToInter
    , const vector<uint32_t>& interToOuter
) {
    updateArray(interToOuterMain, interToOuter);
    updateArrayMapCopy(outerToInterMain, outerToInter);
}

uint64_t CNF::mem_used_longclauses() const
{
    uint64_t mem = 0;
    mem += cl_alloc.mem_used();
    mem += longIrredCls.capacity()*sizeof(ClOffset);
    mem += longRedCls.capacity()*sizeof(ClOffset);
    return mem;
}

uint64_t CNF::print_mem_used_longclauses(const size_t totalMem) const
{
    uint64_t mem = mem_used_longclauses();
    print_stats_line("c Mem for longclauses"
        , mem/(1024UL*1024UL)
        , "MB"
        , stats_line_percent(mem, totalMem)
        , "%"
    );

    return mem;
}

bool CNF::redundant(const Watched& ws) const
{
    return (   (ws.isBin() && ws.red())
            || (ws.isTri()   && ws.red())
            || (ws.isClause()
                && cl_alloc.ptr(ws.get_offset())->red()
                )
    );
}

bool CNF::redundant_or_removed(const Watched& ws) const
{
    if (ws.isBin() || ws.isTri()) {
        return ws.red();
    }

   assert(ws.isClause());
   const Clause* cl = cl_alloc.ptr(ws.get_offset());
   return cl->red() || cl->getRemoved();
}

size_t CNF::cl_size(const Watched& ws) const
{
    switch(ws.getType()) {
        case watch_binary_t:
            return 2;

        case CMSat::watch_tertiary_t:
            return 3;

        case watch_clause_t: {
            const Clause* cl = cl_alloc.ptr(ws.get_offset());
            return cl->size();
        }

        default:
            assert(false);
            return 0;
    }
}

string CNF::watches_to_string(const Lit lit, watch_subarray_const ws) const
{
    std::stringstream ss;
    for(Watched w: ws) {
        ss << watched_to_string(lit, w) << " --  ";
    }
    return ss.str();
}

string CNF::watched_to_string(Lit otherLit, const Watched& ws) const
{
    std::stringstream ss;
    switch(ws.getType()) {
        case watch_binary_t:
            ss << otherLit << ", " << ws.lit2();
            if (ws.red()) {
                ss << "(red)";
            }
            break;

        case CMSat::watch_tertiary_t:
            ss << otherLit << ", " << ws.lit2() << ", " << ws.lit3();
            if (ws.red()) {
                ss << "(red)";
            }
            break;

        case watch_clause_t: {
            const Clause* cl = cl_alloc.ptr(ws.get_offset());
            for(size_t i = 0; i < cl->size(); i++) {
                ss << (*cl)[i];
                if (i + 1 < cl->size())
                    ss << ", ";
            }
            if (cl->red()) {
                ss << "(red)";
            }
            break;
        }

        default:
            assert(false);
            break;
    }

    return ss.str();
}

bool ClauseSizeSorter::operator () (const ClOffset x, const ClOffset y)
{
    Clause* cl1 = cl_alloc.ptr(x);
    Clause* cl2 = cl_alloc.ptr(y);
    return (cl1->size() < cl2->size());
}

void CNF::remove_tri_but_lit1(
    const Lit lit1
    , const Lit lit2
    , const Lit lit3
    , const bool red
    , int64_t& timeAvailable
) {
    //Remove tri
    Lit lits[3];
    lits[0] = lit1;
    lits[1] = lit2;
    lits[2] = lit3;
    std::sort(lits, lits+3);
    timeAvailable -= watches[lits[0]].size();
    timeAvailable -= watches[lits[1]].size();
    timeAvailable -= watches[lits[2]].size();
    removeTriAllButOne(watches, lit1, lits, red);

    //Update stats for tri
    if (red) {
        binTri.redTris--;
    } else {
        binTri.irredTris--;
    }
}

size_t CNF::mem_used_renumberer() const
{
    size_t mem = 0;
    mem += interToOuterMain.capacity()*sizeof(uint32_t);
    mem += outerToInterMain.capacity()*sizeof(uint32_t);
    mem += outer_to_with_bva_map.capacity()*sizeof(uint32_t);
    return mem;
}


vector<lbool> CNF::map_back_to_without_bva(const vector<lbool>& val) const
{
    vector<lbool> ret;
    assert(val.size() == nVarsOuter());
    ret.reserve(nVarsOutside());
    for(size_t i = 0; i < nVarsOuter(); i++) {
        if (!varData[map_outer_to_inter(i)].is_bva) {
            ret.push_back(val[i]);
        }
    }
    assert(ret.size() == nVarsOutside());
    return ret;
}

vector<uint32_t> CNF::build_outer_to_without_bva_map() const
{
    vector<uint32_t> ret;
    size_t at = 0;
    for(size_t i = 0; i < nVarsOuter(); i++) {
        if (!varData[map_outer_to_inter(i)].is_bva) {
            ret.push_back(at);
            at++;
        } else {
            ret.push_back(var_Undef);
        }
    }

    return ret;
}

size_t CNF::mem_used() const
{
    size_t mem = 0;
    mem += sizeof(conf);
    mem += sizeof(binTri);
    mem += seen.capacity()*sizeof(uint16_t);
    mem += seen2.capacity()*sizeof(uint8_t);
    mem += toClear.capacity()*sizeof(Lit);

    return mem;
}

void CNF::save_state(SimpleOutFile& f) const
{
    /*assert(!seen.empty());
    assert(!varData.empty());
    assert(watches.size() != 0);*/

    f.put_vector(interToOuterMain);
    f.put_vector(outerToInterMain);

    f.put_vector(assigns);
    f.put_vector(varData);
    f.put_uint32_t(minNumVars);
    f.put_uint32_t(num_bva_vars);
    f.put_uint32_t(ok);
}

void CNF::load_state(SimpleInFile& f)
{
    assert(seen.empty());
    assert(varData.empty());
    assert(watches.size() == 0);

    f.get_vector(interToOuterMain);
    f.get_vector(outerToInterMain);
    build_outer_to_without_bva_map();

    f.get_vector(assigns);
    f.get_vector(varData);
    minNumVars = f.get_uint32_t();
    num_bva_vars = f.get_uint32_t();
    ok = f.get_uint32_t();

    watches.resize(nVars()*2);
}


void CNF::test_all_clause_attached() const
{
#ifdef DEBUG_ATTACH_MORE
    for (vector<ClOffset>::const_iterator
        it = longIrredCls.begin(), end = longIrredCls.end()
        ; it != end
        ; ++it
    ) {
        assert(normClauseIsAttached(*it));
    }
#endif
}

bool CNF::normClauseIsAttached(const ClOffset offset) const
{
    bool attached = true;
    const Clause& cl = *cl_alloc.ptr(offset);
    assert(cl.size() > 3);

    attached &= findWCl(watches[cl[0]], offset);
    attached &= findWCl(watches[cl[1]], offset);

    return attached;
}

void CNF::find_all_attach() const
{
#ifdef SLOW_DEBUG
    for (size_t i = 0; i < watches.size(); i++) {
        const Lit lit = Lit::toLit(i);
        for (uint32_t i2 = 0; i2 < watches[lit].size(); i2++) {
            const Watched& w = watches[lit][i2];
            if (!w.isClause())
                continue;

            //Get clause
            Clause* cl = cl_alloc.ptr(w.get_offset());
            assert(!cl->freed());

            //Assert watch correctness
            if ((*cl)[0] != lit
                && (*cl)[1] != lit
            ) {
                std::cerr
                << "ERROR! Clause " << (*cl)
                << " not attached?"
                << endl;

                assert(false);
                std::exit(-1);
            }

            //Clause in one of the lists
            if (!find_clause(w.get_offset())) {
                std::cerr
                << "ERROR! did not find clause " << *cl
                << endl;

                assert(false);
                std::exit(1);
            }
        }
    }

    find_all_attach(longIrredCls);
    for(auto& lredcls: longRedCls) {
        find_all_attach(lredcls);
    }
#endif
}

void CNF::find_all_attach(const vector<ClOffset>& cs) const
{
    for(vector<ClOffset>::const_iterator
        it = cs.begin(), end = cs.end()
        ; it != end
        ; ++it
    ) {
        Clause& cl = *cl_alloc.ptr(*it);
        bool ret = findWCl(watches[cl[0]], *it);
        if (!ret) {
            cout
            << "Clause " << cl
            << " (red: " << cl.red() << ")"
            << " doesn't have its 1st watch attached!"
            << endl;

            assert(false);
            std::exit(-1);
        }

        ret = findWCl(watches[cl[1]], *it);
        if (!ret) {
            cout
            << "Clause " << cl
            << " (red: " << cl.red() << ")"
            << " doesn't have its 2nd watch attached!"
            << endl;

            assert(false);
            std::exit(-1);
        }
    }
}


bool CNF::find_clause(const ClOffset offset) const
{
    for (uint32_t i = 0; i < longIrredCls.size(); i++) {
        if (longIrredCls[i] == offset)
            return true;
    }

    for(auto& lredcls: longRedCls) {
        for (ClOffset off: lredcls) {
            if (off == offset)
                return true;
        }
    }

    return false;
}

void CNF::check_wrong_attach() const
{
#ifdef SLOW_DEBUG
    for(auto& lredcls: longRedCls) {
        for (ClOffset offs: lredcls) {
            const Clause& cl = *cl_alloc.ptr(offs);
            for (uint32_t i = 0; i < cl.size(); i++) {
                if (i > 0)
                    assert(cl[i-1].var() != cl[i].var());
            }
        }
    }
#endif
}

uint64_t CNF::count_lits(
    const vector<ClOffset>& clause_array
    , const bool red
    , const bool allowFreed
) const {
    uint64_t lits = 0;
    for(vector<ClOffset>::const_iterator
        it = clause_array.begin(), end = clause_array.end()
        ; it != end
        ; ++it
    ) {
        const Clause& cl = *cl_alloc.ptr(*it);
        if (cl.freed()) {
            assert(allowFreed);
        } else {
            if ((cl.red() ^ red) == false) {
                lits += cl.size();
            }
        }
    }

    return lits;
}

void CNF::print_all_clauses() const
{
    for(vector<ClOffset>::const_iterator
        it = longIrredCls.begin(), end = longIrredCls.end()
        ; it != end
        ; ++it
    ) {
        Clause* cl = cl_alloc.ptr(*it);
        cout
        << "Normal clause offs " << *it
        << " cl: " << *cl
        << endl;
    }


    uint32_t wsLit = 0;
    for (watch_array::const_iterator
        it = watches.begin(), end = watches.end()
        ; it != end
        ; ++it, wsLit++
    ) {
        Lit lit = Lit::toLit(wsLit);
        watch_subarray_const ws = *it;
        cout << "watches[" << lit << "]" << endl;
        for (const Watched *it2 = ws.begin(), *end2 = ws.end()
            ; it2 != end2
            ; it2++
        ) {
            if (it2->isBin()) {
                cout << "Binary clause part: " << lit << " , " << it2->lit2() << endl;
            } else if (it2->isClause()) {
                cout << "Normal clause offs " << it2->get_offset() << endl;
            } else if (it2->isTri()) {
                cout << "Tri clause:"
                << lit << " , "
                << it2->lit2() << " , "
                << it2->lit3() << endl;
            }
        }
    }
}
