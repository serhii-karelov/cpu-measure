## Measure number of cycles from Python
Mesures the number of cycles from start untill the end of the exectuion of a 
code block. Corrects the result for the overhead from measurement. 

Usage: 
```python
from measure import Measure 
measure = Measure()
with measure:
    find_kth_prime(k)
    # ...

print(f"Cycles taken: {measure.cycles}")
# Cycles taken: 314159265358
```

## Details
The library uses `RDTSCP` instaruction to read the TSP register.
Note that the number of cycles is counted from the start utill the end
of the execution of the code block. One should be careful when interpreting
results of long-running tasks, especially that invlole IO. The current
thread can be put to sleep by the OS, and another thread may be scheduled.
In the meatime the TSP register will be being kept inctemented, and this will
be refleted in the measurement. For CPU bound tasks measruments are correct, 
again, unless a thread has been rescheduled. 

## Building
Build requirements:
- `setuptools`

Build with pip (installs build-time requirements automatically)
```
pip install .
```

## Testing
The library must be built first.

```
python -m test
```
