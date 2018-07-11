
int Chunk = 10;

int residue_pair_extended_list ( int s1, int r1, int s2, int r2 ) {
	if ( r1<=0 || r2<=0) {return 0;}
	
	auto it = CL->scores[s1][s2].find(make_pair_un(r1,r2));
	if(it != CL->scores[s1][s2].end()) {
		return it->second;
	}
	
	double score=0, max_score=0, max_val=0;
	int t_s, t_r;
	static int **hasch, max_len;
	
	if ( !hasch || max_len!=(CL->S)->max_len) {
	   max_len=(CL->S)->max_len;
	   if ( hasch) free_int ( hasch, -1);
	   hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	}

	if(CL->cache[s1][r1].first==0) {
		std::stringstream s;
		//s << "select s2,r2,w from ppcas." << CL->seq_name2 << "_bdtc where key = ('" << s1+1 << " " << r1 << "','" << s2+1 << " " << r2 << "')";
		s << "select r1,s2,r2,w from ppcas." << CL->seq_name2 << " where key = '" << s1 << " " << r1/CL->qChunk << "'";
		execute_query(s.str().c_str(), s1, r1/CL->qChunk, CL);
	}

	if(CL->cache[s2][r2].first==0) {
		std::stringstream s;
		s << "select r1,s2,r2,w from ppcas." << CL->seq_name2 << " where key = '" << s2 << " " << r2/CL->qChunk << "'";
		execute_query(s.str().c_str(), s2, r2/CL->qChunk, CL);
	}

	for (int i=0; i<CL->cache[s1][r1].first;i++) {
	    t_s=CL->cache[s1][r1].second[i].s2;
	    t_r=CL->cache[s1][r1].second[i].r2;
	    hasch[t_s][t_r]=CL->cache[s1][r1].second[i].w;
	    max_score+=CL->cache[s1][r1].second[i].w;
	}

	hasch[s1][r1]=FORBIDEN;
	for (int i=0; i<CL->cache[s2][r2].first;i++) {
	    t_s=CL->cache[s2][r2].second[i].s2;
	    t_r=CL->cache[s2][r2].second[i].r2;
	    
	    if (hasch[t_s][t_r]) {
			if (hasch[t_s][t_r]==FORBIDEN) {
				score+=CL->cache[s2][r2].second[i].w;
				max_score+=CL->cache[s2][r2].second[i].w;
			} else {
				double delta;
				delta=MIN(hasch[t_s][t_r],CL->cache[s2][r2].second[i].w);
		    
				score+=delta;
				max_score-=hasch[t_s][t_r];
				max_score+=delta;
				max_val=MAX(max_val,delta);
			}
	    } else { max_score+=CL->cache[s2][r2].second[i].w;}
	}

	max_score-=hasch[s2][r2];	
	clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);

	if ( max_score==0) {return 0;}
	else if ( CL->normalise) {
	    score=((score*CL->normalise)/max_score)*SCORE_K;
	    if (max_val> CL->normalise) {
			score*=max_val/(double)CL->normalise;
	    }
	}
	
	while(((sizeof(size_t) + sizeof(int) + 8) * score_elements + (sizeof(CL->scores) * (CL->S)->nseq)) > CL->max_mem*0.1) {
		std::pair<int,int> tmp_key = lru_sc.remove();
		score_elements-=CL->scores[tmp_key.first][tmp_key.second].size();
		CL->scores[tmp_key.first][tmp_key.second].clear();
	}

	CL->scores[s1][s2][make_pair_un(r1,r2)]=(int)score;
	lru_sc.insert(std::make_pair(s1,s2));
	score_elements++;
	return (int)score;
}
