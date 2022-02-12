import sys
import re

 
re_float = '([\-\+]?[0-9]*(\.?[0-9]+e?[\-\+]?[0-9]*)?)(ns|ms|us|s|m)'
pattern = re.compile(
  fr'^Timer\(([^\)]+)\): <t> = {re_float}, std = {re_float}, {re_float} <= t <= {re_float} \(n=(\d+)\)'
)
 
results = {}
 
for line in sys.stdin:
    try:
        groups = re.match(pattern, line).groups()
    except:
        raise Exception(line)
    assert len(groups) == 14
    name = groups[0]
    vals = list(map(float, groups[1: -1 : 3]))
    n = int(groups[-1])
    scales = [{'ns': 1e-9, 'us': 1e-6, 'ms': 1e-3, 's': 1, 'm': 60}[spec] for spec in groups[3: : 3]]
    vals = [val * scale for (val, scale) in zip(vals, scales)]
    t, std, low, hi = vals
    
    if name not in results:
        results[name] = [0, 0, 0, 0, 0]
    results[name][0] += t
    results[name][1] += std
    results[name][2] += low
    results[name][3] += hi
    results[name][4] += n

headers = [
    "name",
    "avarage",
    "std",
    "low",
    "high",
    "n"
]
print(";".join(headers))
 
for (k, v) in results.items():
  print(k + ';' + ';'.join(map(str, v)))