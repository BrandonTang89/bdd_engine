"""
Union-Find Disjoint Set Data Structure

>>> u = UnionFind(5)
>>> u.find(0)
0
>>> u.union(0, 3)
True
>>> u.union(0, 3)
False
>>> u.union(0, 2)
True
>>> len(u)
3
>>> u
UnionFind with parents: [0, 1, 0, 0, 4]
"""

class UnionFind:
    __slots__ = ["parent", "rank", "size", "count"]

    def __init__(self, n: int) -> None:
        self.parent: list[int] = [i for i in range(n)]
        self.rank: list[int] = [1 for _ in range(n)]
        self.size: list[int] = [1 for _ in range(n)]
        self.count: int = n

    def find(self, x: int) -> int:
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def union(self, x: int, y: int) -> bool:
        root_x: int = self.find(x)
        root_y: int = self.find(y)

        if root_x == root_y:
            return False

        if self.rank[root_x] < self.rank[root_y]:
            self.parent[root_x] = root_y
            self.size[root_y] += self.size[root_x]
        elif self.rank[root_x] > self.rank[root_y]:
            self.parent[root_y] = root_x
            self.size[root_x] += self.size[root_y]
        else:
            self.parent[root_y] = root_x
            self.size[root_x] += self.size[root_y]
            self.rank[root_x] += 1

        self.count -= 1
        return True

    def is_same_set(self, x: int, y: int) -> bool:
        return self.find(x) == self.find(y)

    def size_of_set(self, x: int) -> int:
        return self.size[self.find(x)]

    def num_disjoint_sets(self) -> int:
        return self.count

    def __len__(self) -> int:
        return self.count

    def __repr__(self) -> str:
        return f"UnionFind with parents: {self.parent}"


if __name__ == "__main__":
    import doctest

    doctest.testmod()