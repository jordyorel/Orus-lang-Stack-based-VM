import time
import math
import random

class Box:
    def __init__(self, value):
        self.value = value

def main():
    # Start time for benchmark
    start = time.time()
    
    # Generate random boxes
    boxes = []
    for i in range(100):
        boxes.append(Box(random.randint(0, 99)))
    
    # Count frequency of tens using a dictionary
    freq = {}
    for box in boxes:
        bucket = box.value // 10
        if bucket in freq:
            freq[bucket] += 1
        else:
            freq[bucket] = 1
    
    # Perform some math heavy work
    total = 0.0
    for i in range(100):
        total += math.sqrt(random.uniform(1.0, 1000.0))
    print(round(total))
    
    # Iterate over the frequency map
    for k in sorted(freq.keys()):
        print(k * 10)
        print(freq[k])
    
    # Print elapsed seconds
    elapsed = time.time() - start
    print(f"Time elapse: {elapsed}")

if __name__ == "__main__":
    main()