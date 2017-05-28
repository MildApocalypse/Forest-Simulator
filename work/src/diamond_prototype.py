import random
from collections import namedtuple


Point = namedtuple('Point', ['x', 'y'])

class Heightmap:
    def __init__(self, seed=0, size=5):
        self.initial = None
        self.seed = seed
        self.size = size
        
        if seed is 0:
            random.seed(None)
        else:
            random.seed(seed)
            
        self.map = [ \
            [self.initial for i in range(0,size)] for i in range(0,size) \
        ]
        
    def getAt(self, point):
        """
        Returns z value at position (x, y)
        
        Reverses the relationship so Cartesian co-ordinates can be used.
        """
        return self.map[point.y][point.x]
    
    def setAt(self, point, z=0):
        """
        Sets the z value at position (x, y)
        
        Reverses the relationship so Cartesian co-ordinates can be used.
        """
        try:
            if self.map[point.y][point.x] is None:
                self.map[point.y][point.x] = z
        except IndexError:
            pass
        
    def randint(self):
        minimum = 0
        maximum = 5
        return random.randint(minimum, maximum)
    
    def generate_corners(self, start, end):
        self.setAt(Point(start, start), self.randint())
        self.setAt(Point(start, end), self.randint())
        self.setAt(Point(end, start), self.randint())
        self.setAt(Point(end, end), self.randint())
        
    def generate_heightmap(self):
        start = 0
        end = self.size-1
        self.generate_corners(start, end)
        
        distance = (end - start)
        square_start = Point(start, start)
        
        while distance > 0:
            for x in range(0, end, distance):
                for y in range(0, end, distance):
                    square_start = Point(x, y)
                    self.calculate_square_center(square_start, distance)
            for x in range(0, end, distance):
                for y in range(0, end, distance):
                    square_start = Point(x, y)
                    center_of_square = Point(square_start.x+(distance//2), square_start.y+(distance//2))
                    self.diamond_step(square_start, distance)
            
            distance //= 2
        
    def calculate_square_center(self, square_start, distance):
        average_of_square = self.get_average_of_square(square_start, distance)
        center_of_square = Point(square_start.x+(distance//2), square_start.y+(distance//2))
        self.setAt(center_of_square, average_of_square)
        
    def calculate_diamond_center(self, diamond_center, distance):
        average_of_diamond = self.get_average_of_diamond(diamond_center, distance)
        self.setAt(diamond_center, average_of_diamond)
                
    def diamond_step(self, square, distance):
        half_distance = (distance//2)
        center_of_square = Point(square.x+half_distance, square.y+half_distance)

        diamond_centers = [
            Point(center_of_square.x-half_distance, center_of_square.y),
            Point(center_of_square.x+half_distance, center_of_square.y),
            Point(center_of_square.x, center_of_square.y-half_distance),
            Point(center_of_square.x, center_of_square.y+half_distance),
        ]
        
        if distance > 0:
            for diamond in diamond_centers:
                self.calculate_diamond_center(diamond, half_distance)
                
    def square_step(self, diamond, distance):
        square_starts = [
            Point(diamond.x-distance, diamond.y-distance),
            Point(diamond.x-distance, diamond.y),
            Point(diamond.x, diamond.y-distance),
            Point(diamond.x, diamond.y),
        ]

        distance = distance // 2
        if distance > 0:
            for square in square_starts:
                if (square.x >= 0 and square.y >= 0) and (square.x < self.size and square.y < self.size):
                    self.calculate_square_center(square, distance)

    def get_average_of_square(self, start, distance):
        values = [
            Point(start.x, start.y),
            Point(start.x, start.y+distance),
            Point(start.x+distance, start.y),
            Point(start.x+distance, start.y+distance),
        ]
        
        return self.average_of_list(values)
    
    def get_average_of_diamond(self, start, distance):
        values = [
            Point(start.x+distance, start.y),
            Point(start.x-distance, start.y),
            Point(start.x, start.y+distance),
            Point(start.x, start.y-distance),
        ]
        
        return self.average_of_list(values)
    
    def is_valid(self, point):
        return (point.x >= 0 and point.y >= 0) and (point.x < self.size and point.y < self.size)
    
    def average_of_list(self, list_to_compute):
        count = 0
        total = 0
        for val in list_to_compute:
            if self.is_valid(val):
                total += self.getAt(val)
                count += 1

        if count > 0:
            return total / count

hm = Heightmap(size=129)
hm.generate_heightmap()
for row in hm.map:
    print(row)