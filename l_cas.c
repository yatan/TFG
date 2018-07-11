#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <vector>

#include <tsl/sparse_map.h>
#include <cassandra.h>

struct Constraint{  
	int s2; 
	int r2; 
	int w;  
};

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

int execute_query(const char* query, int s1, int cr1) {
	int Chunk = 10;

	CassCluster* cs_cluster;
	CassSession* cs_session;
	
	CassError rc = CASS_OK;
	CassFuture* future = NULL;

	cass_int32_t r1;
	cass_int32_t s2;
	cass_int32_t r2;
	cass_int32_t w;

	int iStart = cr1*Chunk;
	int iEnd = iStart + Chunk;

	//static int aval_size = (CL->S)->nseq * ((CL->S)->max_len+1) * sizeof(int);

	std::vector<std::vector<std::pair<int,Constraint *> > > cache;
	std::vector<std::vector<tsl::sparse_map<size_t, int>>> scores;
	std::vector<int> cache_it;
	
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

		if(size==0) {
			for(int i=iStart;i<iEnd;i++) {cache[s1][i].first = -1;}
			cass_result_free(result);
			cass_future_free(future);
			cass_statement_free(statement);
			return 1;
		}
		
		for(int i=iStart;i<iEnd;i++) {
			cache[s1][i].second = (Constraint*) malloc(size * sizeof(Constraint));
		}

		while (cass_iterator_next(iterator)) {
			const CassRow* row = cass_iterator_get_row(iterator);
			cass_value_get_int32(cass_row_get_column_by_name(row, "r1"), &r1);
			cass_value_get_int32(cass_row_get_column_by_name(row, "s2"), &s2);
			cass_value_get_int32(cass_row_get_column_by_name(row, "r2"), &r2);
			cass_value_get_int32(cass_row_get_column_by_name(row, "w"), &w);
			
			cache[s1][r1].second[cache_it[r1]].s2 = s2;
			cache[s1][r1].second[cache_it[r1]].r2 = r2;
			cache[s1][r1].second[cache_it[r1]].w = w;
			cache_it[r1]++;
		}
		cass_result_free(result);
	} else {
		print_error(future);
		cass_future_free(future);
		cass_statement_free(statement);
		return -1;
	}
	
	for(int i=iStart;i<iEnd;i++) {
		cache[s1][i].second = (Constraint*) realloc(cache[s1][i].second, cache_it[i] * sizeof(Constraint));
		cache[s1][i].first = cache_it[i];
		cache_it[i]=0;
		
	}
	cass_future_free(future);
	cass_statement_free(statement);
	return 1;
}


int main()
{
  char s[256];
  int s1, r1, Chunk;
  const char *seq_name2 = "R100";

/* strcpy(s, "select r1,s2,r2,w from ppcas.");*/

/* << seq_name2 << " where key = '" << s1 << " " << r1 / Chunk << "'";*/

/* fprintf(stdout, "Elm: %s\n",s); */

/* execute_query(s, s1, r1 / Chunk); */

  return 0;
}