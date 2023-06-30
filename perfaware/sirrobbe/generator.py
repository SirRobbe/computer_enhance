import sys
import json
import struct
import random
from math import sin, cos, asin, sqrt

def square(number: float) -> float:
    result = number * number
    return result

def deg_to_rad(deg: float) -> float:
    result = 0.01745329238 * deg
    return result

# NOTE(Fabi): EarthRadius is generally expected to be 6372.8
def reference_haversine_distance(x0: float, y0: float, x1: float, y1: float, earth_radius: float) -> float:
    
    lat1 = y0
    lat2 = y1
    lon1 = x0
    lon2 = x1
    
    dLat = deg_to_rad(lat2 - lat1)
    dLon = deg_to_rad(lon2 - lon1)
    lat1 = deg_to_rad(lat1)
    lat2 = deg_to_rad(lat2)
    
    a = square(sin(dLat /2.0)) + cos(lat1) * cos(lat2) * square(sin(dLon / 2))
    c = 2.0 * asin(sqrt(a))
    
    result = earth_radius * c
    return result

if len(sys.argv) == 1:
    print("usage: python generator.py <pair count> <seed> <mode>")
    exit()

coordinate_pairs = int(sys.argv[1])
seed = int(sys.argv[2])
mode = sys.argv[3]

random.seed(seed)

pairs = [{} for _ in range(coordinate_pairs)]
sum = 0

binary = open("results.bin", "wb")

if mode == "normal":

    for pair in pairs:
        pair["x0"] = round(random.uniform(-180.0, 180.0), 4)
        pair["y0"] = round(random.uniform(-90.0, 90.0), 4)
        pair["x1"] = round(random.uniform(-180.0, 180.0), 4)
        pair["y1"] = round(random.uniform(-90.0, 90.0), 4)
        
        d = reference_haversine_distance(pair["x0"], pair["y0"], pair["x1"], pair["y1"], 6372.8)
        bin = struct.pack("f", d)
        binary.write(bin)
        sum += d

elif mode == "cluster":

    cluster_count = 4
    samples_per_cluster = int(coordinate_pairs / cluster_count)

    for index in range(cluster_count):
        x_min = random.uniform(-180.0, 180.0)
        x_max = random.uniform(-180.0, 180.0)

        y_min = random.uniform(-90.0, 90.0)
        y_max = random.uniform(-90.0, 90.0)

        pairs[index]["x0"] = random.uniform(x_min, x_max)
        pairs[index]["y0"] = random.uniform(y_min, y_max)
        pairs[index]["x1"] = random.uniform(x_min, x_max)
        pairs[index]["y1"] = random.uniform(y_min, y_max)
        
        d = reference_haversine_distance(pairs[index]["x0"], pairs[index]["y0"], pairs[index]["x1"], pairs[index]["y1"], 6372.8)
        bin = struct.pack("f", d)
        binary.write(bin)
        sum += d

binary.close()

data = {"pairs": pairs}

with open("data.json", "w") as file:
    json.dump(data, file)

average = sum / coordinate_pairs
print(f"result: {average}")