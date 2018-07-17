#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <vector>
#include <sstream>

#include <tsl/sparse_map.h>
#include <cassandra.h>


#define FORBIDEN -1
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int Chunk = 10;
int nseq = 2;
int max_len = 9999;

struct Constraint{
	int s2;
	int r2;
	int w;
};


std::vector<std::vector<std::pair<int,Constraint *> > > cache;

int **declare_int (int f, int s)
{
  int **r;
  int a;
  r=(int**)malloc ( f * sizeof (int*));
  for (a=0; a<f; a++)
    r[a]=(int*)malloc ( s * sizeof (int*));
  return r;
}

void print_error(CassFuture* future) {
	const char* message;
	size_t message_length;
	cass_future_error_message(future, &message, &message_length);
	fprintf(stderr, "Error: %.*s\n", (int)message_length, message);
}

CassCluster* create_cluster(const char* hosts) {
	CassCluster* cluster = cass_cluster_new();
	cass_cluster_set_contact_points(cluster, hosts);
	return cluster;
}

CassError connect_session(const CassCluster* cluster, CassSession* session) {
	CassError rc = CASS_OK;
	CassFuture* future = cass_session_connect(session, cluster);

	cass_future_wait(future);
	rc = cass_future_error_code(future);
	if (rc != CASS_OK) {
		print_error(future);
	}
	cass_future_free(future);
	return rc;
}

int execute_query(const char* query, int s1, int cr1, int cache_type) {

	CassCluster* cs_cluster;
	CassSession* cs_session;

	CassError rc = CASS_OK;
	CassFuture* future = NULL;

	cass_int32_t r1;
	cass_int32_t s2;
	cass_int32_t r2;
	cass_int32_t w;

	int iStart = cr1 * Chunk;
	int iEnd = iStart + Chunk;

	//static int aval_size = (CL->S)->nseq * ((CL->S)->max_len+1) * sizeof(int);



//	std::vector<std::vector<std::pair<int,Constraint *> > > cache;
	std::vector<std::vector<tsl::sparse_map<size_t, int>>> scores;
	std::vector<int> cache_it;

	cache_it.resize(max_len,0);

	if(!cs_session) {
		cs_session = cass_session_new();
		cs_cluster = create_cluster("127.0.0.1");

		if (connect_session(cs_cluster, cs_session) != CASS_OK) {
			cass_cluster_free(cs_cluster);
			cass_session_free(cs_session);
			return -1;
		}
		//cass_cluster_free(cluster);
		//cass_session_free(session);
	}

	CassStatement* statement = cass_statement_new(query, 0);
	future = cass_session_execute(cs_session, statement);
	cass_future_wait(future);

	rc = cass_future_error_code(future);
	if (rc == CASS_OK) {
		const CassResult* result = cass_future_get_result(future);
		CassIterator* iterator = cass_iterator_from_result(result);
		int size = cass_result_row_count ( result );


		printf("Size: %d\n", size);

		if(size==0) {
			for(int i=iStart;i<iEnd && i<max_len+1;i++) {cache[cache_type][i].first = -1;}
			cass_result_free(result);
			cass_future_free(future);
			cass_statement_free(statement);
			return 1;
		}

		//printf("[%d-%d]\n", iStart, iEnd);
		for(int i=iStart;i<iEnd && i<max_len+1;i++) {
			cache[cache_type][i].second = (Constraint*) malloc(size * sizeof(Constraint));
		}

		while (cass_iterator_next(iterator)) {
			const CassRow* row = cass_iterator_get_row(iterator);
			cass_value_get_int32(cass_row_get_column_by_name(row, "r1"), &r1);
			cass_value_get_int32(cass_row_get_column_by_name(row, "s2"), &s2);
			cass_value_get_int32(cass_row_get_column_by_name(row, "r2"), &r2);
			cass_value_get_int32(cass_row_get_column_by_name(row, "w"), &w);

			//printf("r1: %d\n", r1);
			//printf("%d\n",cache_it[r1]);

			cache[cache_type][r1].second[cache_it[r1]].s2 = s2;
			cache[cache_type][r1].second[cache_it[r1]].r2 = r2;
			cache[cache_type][r1].second[cache_it[r1]].w = w;
			cache_it[r1]++;
		}
		cass_result_free(result);
	} else {
		print_error(future);
		cass_future_free(future);
		cass_statement_free(statement);
		return -1;
	}

	for(int i=iStart;i<iEnd && i<max_len+1;i++) {
		cache[cache_type][i].second = (Constraint*) realloc(cache[cache_type][i].second, cache_it[i] * sizeof(Constraint));
		cache[cache_type][i].first = cache_it[i];
		cache_it[i] = 0;

	}
	cass_future_free(future);
	cass_statement_free(statement);
	return 1;
}


int residue_pair_extended_list ( int s1, int r1, int s2, int r2 ) {

	const char *seq_name2 = "rrm_100";
	// Cache para 2 secuencias
	cache.resize( nseq , std::vector<std::pair<int,Constraint *> >( max_len + 1 ) );


	if ( r1<=0 || r2<=0) {return 0;}

	double score=0, max_score=0, max_val=0;
	int t_s, t_r;
	static int **hasch;

	if ( !hasch ) {
	   hasch = declare_int ( max_len, max_len + 1);
	}



	if(cache[0][r1].first==0) {
		std::stringstream s;
		s << "select r1,s2,r2,w from ppcas." << seq_name2 << " where key = '" << s1 << " " << r1/Chunk << "'";
		execute_query(s.str().c_str(), s1, r1/Chunk, 0);

	}

	if(cache[1][r2].first==0) {
		std::stringstream s;
		s << "select r1,s2,r2,w from ppcas." << seq_name2 << " where key = '" << s2 << " " << r2/Chunk << "'";
		execute_query(s.str().c_str(), s2, r2/Chunk, 1);
	}

	for (int i=0; i<cache[0][r1].first;i++) {
	    t_s = cache[0][r1].second[i].s2;
	    t_r = cache[0][r1].second[i].r2;
	    hasch[t_s][t_r] = cache[0][r1].second[i].w;
	    max_score += cache[0][r1].second[i].w;
		printf("s1: %d r1: %d s2: %d r2: %d w: %d\n",s1, r1, t_s, t_r, hasch[t_s][t_r]);
	}

	hasch[s1][r1] = FORBIDEN;

	for (int i=0; i<cache[1][r2].first;i++) {
	    t_s = cache[1][r2].second[i].s2;
	    t_r = cache[1][r2].second[i].r2;

	    if (hasch[t_s][t_r]) {
			if (hasch[t_s][t_r] == FORBIDEN) {
				score += cache[1][r2].second[i].w;
				max_score += cache[1][r2].second[i].w;
			}
			else {
				double delta;
				delta = MIN(hasch[t_s][t_r], cache[1][r2].second[i].w);

				score += delta;
				max_score -= hasch[t_s][t_r];
				max_score += delta;
				max_val = MAX(max_val,delta);
			}
	    }
		else {
			max_score += cache[1][r2].second[i].w;
		}
	}

	max_score -= hasch[s2][r2];
	//clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);
	int normalise = 1;
	int SCORE_K = 10;
	if ( max_score == 0) {
		return 0;
	}
	else if ( normalise) {
	    score=(((float)score * normalise)/(float)max_score)*SCORE_K;
	    if (max_val > normalise) {
			score *= (double)max_val/(double)normalise;
	    }
	}

	return (int)score;
}



int main()
{
  int s1=0, r1=1, s2=12, r2=2;

  //const char *seq_name2 = "rrm_100";

  int score = residue_pair_extended_list ( s1, r1, s2, r2 );
  printf("Score: %d\n", score);

  return 0;
}
