/*
 * tea.h
 *
 *  Created on: Dec 15, 2012
 *      Author: Carsten Kemena
 *
 *   Copyright 2012 Carsten Kemena
 *
 *
 * This file is part of BioTools.
 *
 * BioTools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BioTools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BioTools.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TEA_H_
#define TEA_H_

// C headers
#include <cstdlib>
#include <cstdio>

// C++ headers
#include <map>
#include <stack>
#include <string>


// boost header
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// other headers
#include <omp.h>


// BioTools headers
#include "../../lib/Clustering/Tree.h"
#include "../../lib/Clustering/clustering.h"
#include "../../lib/Clustering/kmeans.h"
#include "../../lib/HMM/fw_bw.h"
#include "../../lib/Sequence/gotoh.h"
#include "../../lib/Sequence/aln_analysis.h"
#include "../../lib/utils/Library.h"
#include "../../lib/utils/Matrix.h"
#include "../../lib/utils/ScoringMatrix.h"



struct Tree_opts
{
	std::string tree_i_f;
	std::string tree_o_f;
	std::string dist_mat_method;
	std::string tree_method;
	std::string alphabet;
	unsigned short tree_k;
	unsigned short cluster_size;

};

struct Aln_opts
{
	std::string pair_aln_method;
	int gapopen;
	int gapext;
	std::string matrix_name;
	BioTools::Utils::Scoring_Matrix score_mat;
};

BioTools::Seq::Alignment *
progressive_aln(BioTools::Clustering::TreeNode *root, BioTools::Utils::Library &lib, std::vector<BioTools::Seq::Alignment*> alns);

void
prod_seq_aln(BioTools::Seq::SequenceSet &seq_set, Tree_opts &tree_opts, Aln_opts &aln_opts, BioTools::Seq::Alignment &result_aln);


#endif /* TEA_H_ */
