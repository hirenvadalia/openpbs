import json

moms = [100, 25]
ncpus = [200, 100, 50]
asyncdb = [False, True]
jobtype = ['j', 'ja']
vnodes = 0

setups = {}

for _m in moms:
	for _c in ncpus:
		tot = _m * _c
		if tot == 5000:
			for _d in asyncdb:
				for _j in jobtype:
					_n = str(_m) + 'm' + str(_c) + 'cpu'
					_n += '_' + ('async' if _d else 'sync')
					_n += '_' + _j
					_t = {
						"total_num_svrs": 1,
						"total_num_moms": _m,
						"num_moms_per_host": 0,
						"num_cpus_per_mom": _c,
						"num_vnodes_per_mom": vnodes,
						"async_db": _d,
						"job_type": _j
					}
					if _j == 'ja':
						_t['total_num_jobs'] = 100
						_t['num_subjobs'] = 1000
					else:
						_t['total_num_jobs'] = 100000
						_t['num_subjobs'] = 0
					setups.setdefault(_n, _t)

print(json.dumps({'setups': setups}, indent=2))
