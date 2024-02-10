# Reproducing BloomStore

BloomStore Instance (for each Partition)

- RAM: Active Bloom Buffer (Bloom Filter + Table)
- SSD: Log Structured Bloom Buffers

BloomStore Methods: 

- Get(`k`)
    1. Skim the active bloom buffer for key `k`. 
    2. Skim the chain of bloom buffers on SSD
        Q: Layout like:
        ```
        (Bloom Filter)*N + (Table)*N
        ```
        or
        ```
        (Bloom Filter + Table)*N
        ```
        or
        ```
        ((Bloom Filter)*K + (Table)*K)*M
        ```
        where `(Bloom Filter)*K` can be held in memory?
        (It looks like the first one to me)
    3. Return latest possible value for key `k`. 

- Put(`k`, `v`)
    1. Update active bloom filter. 
    2. Insert into active hash map.
        Q: Lock how?
    3. If active buffer is full, flush it to SSD. 