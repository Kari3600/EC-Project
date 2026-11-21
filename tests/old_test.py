RANDOM_RUNS = 200
RANDOM_STARTS = 200

def generate():
    yield ("Random", [[[], ("RandomSolver", {"seed": seed})] for seed in range(RANDOM_RUNS)])
    yield ("Nearest Neighbor I", [[[start], ("NearestNeighborI", {})] for start in range(RANDOM_STARTS)])


    yield ("Nearest Neighbor II", [[[start], ("NearestNeighborII", {})] for start in range(RANDOM_STARTS)])
    yield ("Greedy Cycle", [[[start], ("GreedyCycle", {})] for start in range(RANDOM_STARTS)])
    yield ("Nearest Neighbor II with 2regret", [[[start], ("NearestNeighborII_2regret", {"alpha": 1.0})] for start in range(RANDOM_STARTS)])
    yield ("Nearest Neighbor II with weighted 2regret", [[[start], ("NearestNeighborII_2regret", {"alpha": 0.5})] for start in range(RANDOM_STARTS)])
    yield ("Greedy Cycle with 2regret", [[[start], ("GreedyCycle_2regret", {"alpha": 1.0})] for start in range(RANDOM_STARTS)])
    yield ("Greedy Cycle with weighted 2regret", [[[start], ("GreedyCycle_2regret", {"alpha": 0.5})] for start in range(RANDOM_STARTS)])

    for search in ["Greedy", "Steepest"]:
        for intra in ["Nodes", "Edges"]:
            yield (f"Local Search {search}, intra {intra.lower()}, random start", [[[], ("RandomSolver", {"seed": seed}), ("LocalSearch", {"searchType": search, "intraMove": intra})] for seed in range(RANDOM_RUNS)])
            yield (f"Local Search {search}, intra {intra.lower()}, greedy start", [[[start], ("GreedyCycle_2regret", {}), ("LocalSearch", {"searchType": search, "intraMove": intra})] for start in range(RANDOM_STARTS)])

    yield ("Local Search with candidates", [[[], ("RandomSolver", {"seed": seed}), ("LocalSearchWithCandidates", {"candidatesCount": 10})] for seed in range(RANDOM_RUNS)])
    yield ("Local Search with list of moves", [[[], ("RandomSolver", {"seed": seed}), ("ListOfMovesSolver", {})] for seed in range(RANDOM_RUNS)])