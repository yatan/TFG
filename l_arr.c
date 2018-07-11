#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <vector>

#include <tsl/sparse_map.h>

int Chunk = 10;

int residue_pair_extended_list ( int s1, int r1, int s2, int r2 ) {
	if ( r1<=0 || r2<=0) {return 0;}
	
	double score=0, max_score=0, max_val=0;
	int t_s, t_r;
	static int **hasch, max_len;
	
	/*if ( !hasch ) {
	   hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	}*/

	if(cache[s1][r1].first==0) {
		//std::stringstream s;
		//s << "select r1,s2,r2,w from ppcas." << CL->seq_name2 << " where key = '" << s1 << " " << r1/CL->qChunk << "'";
		//execute_query(s.str().c_str(), s1, r1/CL->qChunk, CL);
	}

	if(cache[s2][r2].first==0) {
		//std::stringstream s;
		//s << "select r1,s2,r2,w from ppcas." << CL->seq_name2 << " where key = '" << s2 << " " << r2/CL->qChunk << "'";
		//execute_query(s.str().c_str(), s2, r2/CL->qChunk, CL);
	}

	for (int i=0; i<cache[s1][r1].first;i++) {
	    t_s = cache[s1][r1].second[i].s2;
	    t_r = cache[s1][r1].second[i].r2;
	    hasch[t_s][t_r] = cache[s1][r1].second[i].w;
	    max_score += cache[s1][r1].second[i].w;
	}

	hasch[s1][r1]=FORBIDEN;
	
	for (int i=0; i<cache[s2][r2].first;i++) {
	    t_s = cache[s2][r2].second[i].s2;
	    t_r = cache[s2][r2].second[i].r2;
	    
	    if (hasch[t_s][t_r]) {
			if (hasch[t_s][t_r] == FORBIDEN) {
				score += cache[s2][r2].second[i].w;
				max_score += cache[s2][r2].second[i].w;
			} 
			else {
				double delta;
				delta = MIN(hasch[t_s][t_r], cache[s2][r2].second[i].w);
		    
				score += delta;
				max_score -= hasch[t_s][t_r];
				max_score += delta;
				max_val = MAX(max_val,delta);
			}
	    } 
		else { 
			max_score+=CL->cache[s2][r2].second[i].w;
		}
	}

	max_score -= hasch[s2][r2];	
	//clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);

	if ( max_score == 0) {
		return 0;
	}
	/*else if ( CL->normalise) {
	    score=((score*CL->normalise)/max_score)*SCORE_K;
	    if (max_val> CL->normalise) {
			score*=max_val/(double)CL->normalise;
	    }
	}*/
	
	//CL->scores[s1][s2][make_pair_un(r1,r2)]=(int)score;

	return (int)score;
}
