/*
 * Library.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Carsten Kemena
 *
 * This file is part of BioTools++.
 *
 * BioTools++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BioTools++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BioTools++.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "Library.h"

namespace BioTools {
namespace Utils {


using namespace std;
using namespace BioTools::Seq;

Library::Library(const SequenceSet &set):_n_seqs(set.n_seqs()),_max_length(0)
{
	_pairs.reset(new std::vector<std::vector<Match_point> >);
	_n_pairs=(_n_seqs*(_n_seqs-1))/2;
	_pairs->resize(_n_pairs);
	for (size_t i = 0; i<_n_seqs; ++i)
	{
		printf("%i\n", i);
		if (set[i].size() > _max_length)
			_max_length = set[i].size();
	}
}

Library::~Library() {
	// TODO Auto-generated destructor stub
}


void
Library::add(unsigned int seq1_id, unsigned int seq2_id, unsigned int pos1, unsigned int pos2)
{
	// the index of a pair i/j (with i<j) is calculated as following: i*n_seq - (i*(i+1)/2) +j-i-1
	// i*n_seq - (i*(i+1)/2) gives the first index having i at the beginning
	// j-i-1 adds the shift necessary for the pair i/j
	unsigned int id;
	if (seq1_id < seq2_id)
	{
		id = seq1_id*_n_seqs - (seq1_id*(seq1_id+1)/2) +seq2_id-seq1_id-1;
		(*_pairs)[id].push_back(Match_point(pos1,pos2,1.0));
	}
	else
	{
		id = seq2_id*_n_seqs - (seq2_id*(seq2_id+1)/2) +seq1_id-seq2_id-1;
		(*_pairs)[id].push_back(Match_point(pos2,pos1,1.0));
	}
}


void
Library::add(unsigned int seq1_id, unsigned int seq2_id, unsigned int pos1, unsigned int pos2, float score)
{
	// the index of a pair i/j (with i<j) is calculated as following: i*n_seq - (i*(i+1)/2) +j-i-1
	// i*n_seq - (i*(i+1)/2) gives the first index having i at the beginning
	// j-i-1 adds the shift necessary for the pair i/j
	unsigned int id;
	if (seq1_id < seq2_id)
	{
		id = seq1_id*_n_seqs - (seq1_id*(seq1_id+1)/2) +seq2_id-seq1_id-1;
		(*_pairs)[id].push_back(Match_point(pos1, pos2, score));
	}
	else
	{
		id = seq2_id*_n_seqs - (seq2_id*(seq2_id+1)/2) +seq1_id-seq2_id-1;
		(*_pairs)[id].push_back(Match_point(pos2, pos1, score));
	}
}

void
Library::reserve_add_memory(unsigned int seq1_id, unsigned int seq2_id, size_t n_entries)
{
	unsigned int id;
	if (seq1_id < seq2_id)
		id = seq1_id*_n_seqs - (seq1_id*(seq1_id+1)/2) +seq2_id-seq1_id-1;
	else
		id = seq2_id*_n_seqs - (seq2_id*(seq2_id+1)/2) +seq1_id-seq2_id-1;
	(*_pairs)[id].reserve((*_pairs)[id].size()+n_entries);
}


void
Library::get(size_t seq1_id, size_t seq2_id, map<Match, int> &match_points) const
{
	match_points.clear();
	vector<Match_point>::const_iterator it1, it2, it1_end, it2_end;
	unsigned int small, large, small_id, large_id;
	bool is_smaller = seq1_id < seq2_id;

	small = min(seq1_id, seq2_id);
	large = max(seq1_id, seq2_id);

	size_t i, j, vec_len;
	typedef pair<unsigned int, int> score;
	vector<score> *match_field = new vector<score>[_max_length];
	unsigned int k, pos1, pos2;
	int value;

	// add information from direct pairwise alignment
	// the index of a pair i/j (with i<j) is calculated as following: i*n_seq - (i*(i+1)/2) +j-i-1
	// i*n_seq - (i*(i+1)/2) gives the first index having i at the beginning
	// j-i-1 adds the shift necessary for the pair i/j
	small_id = small*_n_seqs - (small*(small+1)/2) +large-small-1;
	it1     = (*_pairs)[small_id].begin();
	it1_end = (*_pairs)[small_id].end();
	if (is_smaller)
	{
		while (it1 != it1_end)
		{
			match_points[Match(it1->x, it1->y)] += it1->score;
			++it1;
		}
	}
	else
	{
		while (it1 != it1_end)
		{
			match_points[Match(it1->y, it1->x)] += it1->score;
			++it1;
		}
	}

	// add triangular information
	for (i=0; i<_n_seqs; ++i)
	{
		if ((i==small) || (i==large))
			continue;
		for (j=0; j<_max_length; ++j)
			match_field[j].clear();

		if (i<small)
		{
			small_id = i*_n_seqs - (i*(i+1)/2) +small-i-1;
			it1     = (*_pairs)[small_id].begin();
			it1_end = (*_pairs)[small_id].end();
			while (it1 != it1_end)
			{
				match_field[it1->x].push_back(score(it1->y, it1->score));

				++it1;
			}
		}
		else
		{
			small_id = small*_n_seqs - (small*(small+1)/2) +i-small-1;
			it1     = (*_pairs)[small_id].begin();
			it1_end = (*_pairs)[small_id].end();
			while (it1 != it1_end)
			{
				match_field[it1->y].push_back(score(it1->x, it1->score));
				++it1;
			}
		}

		if (i<large)
		{
			large_id = i*_n_seqs - (i*(i+1)/2) +large-i-1;
			it2     = (*_pairs)[large_id].begin();
			it2_end = (*_pairs)[large_id].end();
			while (it2 != it2_end)
			{
				pos1 = it2->x;
				pos2 = it2->y;
				value = it2->score;
				vec_len = match_field[pos1].size();
				for (k=0; k<vec_len; ++k)
				{
					if (is_smaller)
						match_points[Match(match_field[pos1][k].first, pos2)] += min(value,match_field[pos1][k].second);
					else
						match_points[Match(pos2, match_field[pos1][k].first)] += min(value,match_field[pos1][k].second);
				}
				++it2;
			}
		}
		else
		{
			large_id = large*_n_seqs - (large*(large+1)/2) +i-large-1;
			it2     = (*_pairs)[large_id].begin();
			it2_end = (*_pairs)[large_id].end();
			while (it2 != it2_end)
			{
				pos1 = it2->y;
				pos2 = it2->x;
				value = it2->score;
				vec_len = match_field[pos1].size();
				for (k=0; k<vec_len; ++k)
				{
					if (is_smaller)
						match_points[Match(match_field[pos1][k].first, pos2)] += min(value,match_field[pos1][k].second);
					else
						match_points[Match(pos2, match_field[pos1][k].first)] += min(value,match_field[pos1][k].second);
				}
				++it2;
			}
		}

	}

	map<Match, int>::iterator it3, it3_end;
	it3_end = match_points.end();
	for (it3 = match_points.begin(); it3 != it3_end; ++it3)
		it3->second *=1000.0/_n_seqs;
	delete[] match_field;
}


void Library::get(const BioTools::Seq::Alignment &aln1, const BioTools::Seq::Alignment &aln2, std::map<Match, int> &match_points) const
{

	match_points.clear();
	size_t n_seq1 = aln1.n_seqs();
	size_t n_seq2 = aln2.n_seqs();
	size_t i,j,k,pos;
	map<Match, int> tmp_match_points;
	map<Match, int>::iterator it, it_end;
	vector<unsigned int> convert1, convert2;
	convert1.resize(aln1.size());
	convert2.resize(aln2.size());
	pair<map<Match, int>::iterator,bool> ret;
	size_t last_id1 = -1;
	size_t last_id2 = -1;
	for (i=0; i<n_seq1; ++i)
	{
		const Sequence &seq1 = aln1[i];
		pos=0;
		if (last_id1 == seq1.aln_id())
			continue;
		last_id1=seq1.aln_id();
		for (k=0; k<aln1.size(); ++k)
		{
			if (seq1[k] != '-')
				convert1[pos++] = k;
		}

		last_id2=-1;
		for (j=0; j<n_seq2; ++j)
		{
			const Sequence &seq2 = aln2[j];
			if (last_id2 == seq2.aln_id())
				continue;
			last_id2=seq2.aln_id();
			pos=0;

			for (k=0; k<aln2.size(); ++k)
			{
				if (seq2[k] != '-')
					convert2[pos++] = k;

			}

			get(last_id1, last_id2, tmp_match_points);
			it_end=tmp_match_points.end();
			for (it=tmp_match_points.begin(); it!=it_end; ++it)
			{
				Match tmp_m(convert1[it->first.first], convert2[it->first.second]);
				ret=match_points.insert(pair<Match, int>(tmp_m, it->second));

				if (!ret.second)
					ret.first->second+=it->second;
			}
		}
	}
	it_end = match_points.end();
	size_t overall=n_seq1+n_seq2;
	for (it = match_points.begin(); it != it_end; ++it )
	{
		it->second/=overall;
		//printf("%i ", it->second);
	}
}


void
Library::get(const BioTools::Seq::Alignment &aln, const BioTools::Seq::Sequence &seq, std::map<Match, int> &match_points) const
{
	match_points.clear();
	size_t n_seq = aln.n_seqs();
	size_t i,k,pos;

	map<Match, int> tmp_match_points;
	map<Match, int>::iterator it, it_end;
	vector<unsigned int> convert;
	convert.resize(aln.size());
	pair<map<Match, int>::iterator,bool> ret;


	for (i=0; i<n_seq; ++i)
	{
		const Sequence &seq1 = aln[i];
		pos=0;
		for (k=0; k<aln.size(); ++k)
		{
			if (seq1[k] != '-')
				convert[pos++] = k;
		}

		get(aln[i].id(), seq.id(), tmp_match_points);
		it_end=tmp_match_points.end();
		for (it=tmp_match_points.begin(); it!=it_end; ++it)
		{
			Match tmp_m(convert[it->first.first], it->first.second);
			ret=match_points.insert(pair<Match, int>(tmp_m, it->second));
			if (!ret.second)
				ret.first->second+=it->second;
		}
	}

	it_end = match_points.end();
	size_t overall=n_seq+1;
	for (it = match_points.begin(); it != it_end; ++it )
		it->second/=overall;

}

void
Library::sort()
{
	for (size_t i=0; i<_n_pairs; ++i)
		std::sort((*_pairs)[i].begin(),(*_pairs)[i].end());
}


void
Library::relax()
{
	//this->sort();

	omp_lock_t my_lock;

	omp_init_lock(&my_lock);


	/*for (i=0; i<_n_seqs; ++i)
		(*new_vals)[i].insert((*new_vals)[i].begin(), (*_pairs)[i].begin(), (*_pairs)[i].end());
*/
	std::vector<std::vector<Match_point> > *new_vals = new std::vector<std::vector<Match_point> >();
	new_vals->resize(_n_pairs);
  #pragma omp parallel shared(new_vals)
  {



		Matrix<float> final_mat(_max_length, _max_length);
		Matrix<float> tmp_mat(_max_length, _max_length*2);
		vector<pair<int,int> > used;
		used.resize(_max_length*_max_length);
		vector<int> n_hits(_max_length*2, 0);
		size_t use;

		std::vector<Match_point>::const_iterator it,it_end;

		size_t i,j,k,id, current_pair_id, pos;
		int l;
 	 #pragma omp for nowait
	for (i=0; i<_n_seqs; ++i)
	{
		for (j=i+1; j<_n_seqs; ++j)
		{
			current_pair_id=i*_n_seqs - (i*(i+1)/2) +j-i-1;
			it_end=(*_pairs)[current_pair_id].end();
			use=0;
			for (it=(*_pairs)[current_pair_id].begin(); it!=it_end; ++it)
			{
				final_mat[it->x][it->y]+=2*it->score;
				used[use].first=it->x;
				used[use].second=it->y;
				++use;
			}

			for (k=0; k<_n_seqs; ++k)
			{
				if ((k==i) || (k==j))
					continue;
				if (k<i)
				{

					id=k*_n_seqs - (k*(k+1)/2) +i-k-1;
					it=(*_pairs)[id].begin();
					it_end=(*_pairs)[id].end();
					for (it=(*_pairs)[id].begin(); it!=it_end; ++it)
					{
						pos=n_hits[it->x]*2;
						++n_hits[it->x];
						tmp_mat[it->x][pos]=it->y;
						tmp_mat[it->x][++pos]=it->score;
					}
				}
				else
				{
					id=i*_n_seqs - (i*(i+1)/2) +k-i-1;
					it=(*_pairs)[id].begin();
					it_end=(*_pairs)[id].end();
					for (it=(*_pairs)[id].begin(); it!=it_end; ++it)
					{
						pos=n_hits[it->y]*2;
						++n_hits[it->y];
						tmp_mat[it->y][pos]=it->x;
						tmp_mat[it->y][++pos]=it->score;
					}
				}
				if (k<j)
				{
					id=k*_n_seqs - (k*(k+1)/2) +i-k-1;
					it=(*_pairs)[id].begin();
					it_end=(*_pairs)[id].end();
					for (it=(*_pairs)[id].begin(); it!=it_end; ++it)
						for(l=0; l<n_hits[it->x]; ++l)
							if (final_mat[tmp_mat[it->x][l*2]][it->y])
								final_mat[tmp_mat[it->x][l*2]][it->y]+= tmp_mat[it->x][l*2+1]* it->score;
				}
				else
				{
					id=i*_n_seqs - (i*(i+1)/2) +k-i-1;
					it=(*_pairs)[id].begin();
					it_end=(*_pairs)[id].end();
					for (it=(*_pairs)[id].begin(); it!=it_end; ++it)
						for(l=0; l<n_hits[it->y]; ++l)
							if (final_mat[tmp_mat[it->y][l*2]][it->x])
								final_mat[tmp_mat[it->y][l*2]][it->x]+= tmp_mat[it->y][l*2+1]* it->score;
				}
				for (k=0; k<2*_max_length; ++k)
					n_hits[k]=0;
			}

			for (k=0; k<use; ++k)
			{
				omp_set_lock(&my_lock);
				(*new_vals)[current_pair_id].push_back(Match_point(used[k].first, used[k].second, (final_mat[used[k].first][used[k].second]/_n_seqs)*100));
				omp_unset_lock(&my_lock);
			}
			use=0;
		}
	}
  }
  omp_destroy_lock(&my_lock);
	_pairs.reset(new_vals);
}


void
Library::print(const BioTools::Seq::SequenceSet &set, std::string out_f)
{
	FILE *out_F = my_fopen(out_f, "w");
	vector<Match_point>::const_iterator it, it_end;
	size_t i, j;

	// add information from direct pairwise alignment
	// the index of a pair i/j (with i<j) is calculated as following: i*n_seq - (i*(i+1)/2) +j-i-1
	// i*n_seq - (i*(i+1)/2) gives the first index having i at the beginning
	// j-i-1 adds the shift necessary for the pair i/j
	unsigned int id;
	for (i=0; i<_n_seqs; ++i)
	{
		for (j=i+1; j<_n_seqs; ++j)
		{
			fprintf(out_F, "#%li %li\n", i, j);
			id = i*_n_seqs - (i*(i+1)/2) +j-i-1;
			it     = (*_pairs)[id].begin();
			it_end = (*_pairs)[id].end();

			while (it != it_end)
			{
				fprintf(out_F, "%u %u %f\n", it->x, it->y, it->score);
				++it;
			}

		}
	}
	fclose(out_F);
}



} /* namespace Utils */
} /* namespace BioTools */
