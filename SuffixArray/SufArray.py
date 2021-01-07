#!/usr/bin/python3


def brute_force(P, T, SA):
    out_file = open("brute.out", "w")
    for pattern in P:
        n = len(T)
        counter = 0
        for i in range(n):
            if pattern in T[SA[i] : n - 1][: len(pattern)]:
                counter += 1
        out_file.write("{}\n".format(counter))
    out_file.close()


binary_compare_time = 0


def binary_search(P, T, SA):
    global binary_compare_time
    out_file = open("binary.out", "w")
    n = len(T)
    for pattern in P:
        # i_s case
        i_s = pattern + "#"
        l = 0
        r = len(SA)
        while l + 1 < r:
            # before = time.time()
            m = (l + r) // 2
            if i_s <= T[SA[m] : n]:
                r = m
            else:
                l = m
            # binary_compare_time = max(binary_compare_time, time.time() - before)

        i_s = l

        # i_e case
        i_e = pattern + "~"
        l = 0
        r = len(SA)
        while l + 1 < r:
            # before = time.time()
            m = (l + r) // 2
            if i_e <= T[SA[m] : n]:
                r = m
            else:
                l = m
            # binary_compare_time = max(binary_compare_time, time.time() - before)

        i_e = l

        out_file.write("{}\n".format(i_e - i_s))
    out_file.close()


def calculate_lcp(P, T):
    n = min(len(P), len(T))
    counter = 0
    for i in range(n):
        if P[i] == T[i]:
            counter += 1
        else:
            break
    return counter


lcp_compare_time = 0
lcp_calc_time = 0


def lcp_search(P, T, SA):
    global lcp_compare_time, lcp_calc_time
    out_file = open("lcp.out", "w")
    n = len(T)
    for pattern in P:
        # i_s case
        i_s = pattern + "#"
        l = 0
        r = len(SA)
        lcp_l = 0
        lcp_r = 0
        k = 0
        while l + 1 < r:
            m = (l + r) // 2
            # before = time.time()
            if i_s[k : len(i_s)] <= T[SA[m] + k : n]:
                r = m
            else:
                l = m
            # lcp_compare_time = max(time.time() - before, lcp_compare_time)
            # before = time.time()
            lcp_l = calculate_lcp(i_s, T[SA[l] : m])
            lcp_r = calculate_lcp(i_s, T[SA[len(SA) - 1 if r == len(SA) else r] : m])
            # lcp_calc_time = max(time.time() - before, lcp_calc_time)
            k = min(lcp_l, lcp_r)

        i_s = l

        # i_e case
        i_e = pattern + "~"
        l = 0
        r = len(SA)
        lcp_l = 0
        lcp_r = 0
        k = 0
        while l + 1 < r:
            m = (l + r) // 2
            # before = time.time()
            if i_e[k : len(i_e)] <= T[SA[m] + k : n]:
                r = m
            else:
                l = m
            lcp_compare_time = max(time.time() - before, lcp_compare_time)
            # before = time.time()
            lcp_l = calculate_lcp(i_e, T[SA[l] : m])
            lcp_r = calculate_lcp(i_e, T[SA[len(SA) - 1 if r == len(SA) else r] : m])
            # lcp_calc_time = max(time.time() - before, lcp_calc_time)
            k = min(lcp_l, lcp_r)

        i_e = l

        out_file.write("{}\n".format(i_e - i_s))
    out_file.close()


inp_file = open("dna.txt", "r")

N = int(inp_file.readline().strip())
T = inp_file.readline().strip()
SA = [int(idx) for idx in inp_file.readline().strip().split()]
LCP = [int(idx) for idx in inp_file.readline().strip().split()]
N_P = int(inp_file.readline().strip())
P = []

while N_P > 0:
    P += [inp_file.readline().strip()]
    N_P -= 1

print("Finish to load")

import time

before = time.time()
brute_force(P, T, SA)
print("brute: {}".format(time.time() - before))
before = time.time()
binary_search(P, T, SA)
print("binary: {}".format(time.time() - before))
before = time.time()
lcp_search(P, T, SA)
print("lcp_search: {}".format(time.time() - before))

# print("{} {}(+{})".format(binary_compare_time, lcp_compare_time, lcp_calc_time))

inp_file.close()
