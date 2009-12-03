#!/usr/bin/ruby
require 'rubygems'
require 'RMagick'
include Magick

def mda(width,height)
	Array.new(width) { Array.new(height) }
end

# Place initial values in our starting square so the methods have something to average
def seedIndices(initial, size)
	# Make our terrain random based on the current Unix epoch time
	seed = Time.now.to_i
	srand(seed.to_i)

	# Instantiate an array of random start values
	iVals=Array.new(4)
	iVals.collect! { |x| rand(initial)-(initial/2.0) }

	# Seed the terrain
	puts "Seeding initial indices with range #{initial} and values #{iVals.inspect}"
	putIndex(0,      0,      iVals[0])
	putIndex(0,      size-1, iVals[1])
	putIndex(size-1, 0,      iVals[2])
	putIndex(size-1, size-1, iVals[3])
end

# The following two functions are to provide a generic interface to the array used to store the heightmap data
# This way, the script can be easily modified to output to a different format
def getIndex(x, y)
	$terrain[x.to_i][y.to_i]
end

def putIndex(x, y, value)
	if $max < value
		$max = value
	end
	if $min > value
		$min = value
	end
	$terrain[x.to_i][y.to_i] = value
end

# The first step in midpoint heightmapping
def calcBox(lowerX, lowerY, upperX, upperY, entropy)
	# Calculate our halfway points
	offset = (upperX - lowerX) / 2.0
	middleX = lowerX + offset
	middleY = lowerY + offset

	# Compute the size of intersection of the diagonals
	# as the average of the four corners plus a random amount
	middle = getIndex(lowerX, lowerY) + getIndex(lowerX, upperY) + getIndex(upperX, lowerY) + getIndex(upperX, upperY)
	middle = middle / 4.0
	middle = middle + rand(entropy) - (entropy/2.0)

	putIndex(middleX, middleY, middle)
end

# The second step in midpoint heightmapping
def calcDiamond(lowerX, lowerY, upperX, upperY, entropy, size)
	# Compute indices
	offset = (upperX - lowerX) / 2.0
	middleX = lowerX + offset
	middleY = lowerY + offset
	preX = lowerX - offset
	postX = upperX + offset
	preY = lowerY - offset
	postY = upperY + offset
	
	midLeft = midRight = midTop = midBottom = -1

	midLeft = getIndex(lowerX, lowerY) + getIndex(lowerX, upperY) + getIndex(middleX, middleY)
	if preX < 0
		midLeft = midLeft * 4.0 / 3.0
	else
		midLeft = midLeft + getIndex(preX, middleY)
	end
	midLeft = midLeft / 4.0
	midLeft = midLeft + rand(entropy) - (entropy/2.0)
	
	midRight = getIndex(upperX, lowerY) + getIndex(upperX, upperY) + getIndex(middleX, middleY)
	if postX >= size
		midRight = midRight * 4.0 / 3.0
	else
		midRight = midRight + getIndex(postX, middleY)
	end
	midRight = midRight / 4.0
	midRight = midRight + rand(entropy) - (entropy/2.0)
	
	midTop = getIndex(lowerX, upperY) + getIndex(upperX, upperY) + getIndex(middleX, middleY)
	if postY >= size
		midTop = midTop * 4.0 / 3.0
	else
		midTop = midTop + getIndex(middleX, postY)
	end
	midTop = midTop / 4.0
	midTop = midTop + rand(entropy) - (entropy/2.0)
	
	midBottom = getIndex(lowerX, lowerY) + getIndex(upperX, lowerY) + getIndex(middleX, middleY)
	if preY < 0
		midBottom = midBottom * 4.0 / 3.0
	else
		midBottom = midBottom + getIndex(middleX, preY)
	end
	midBottom = midBottom / 4.0
	midBottom = midBottom + rand(entropy) - (entropy/2.0)

	putIndex(lowerX, middleY, midLeft)
	putIndex(upperX, middleY, midRight)
	putIndex(middleX, upperY, midTop)
	putIndex(middleX, lowerY, midBottom)
end

def genTerrain(level, size, entropy, granularity)
	if level + 1 < size
		genTerrain(level*2, size, entropy/granularity, granularity)
	else
		seedIndices(entropy/granularity, size)
	end

	# The highest level will be computed first, with 1 segment
	segments = (size - 1) / level
	puts "Computing #{segments} segments for level #{level} with params #{entropy}, #{granularity}"

	# First, compute the midpoints of all the sections
	segments.times do |x|
		segments.times do |y|
			# Divide the grid into segments ** 2 pieces
			calcBox(x*level, y*level, (x+1)*level, (y+1)*level, entropy)
		end
	end

	# Then, use the midpoint calculations to compute the halfway heights for edges
	segments.times do |x|
		segments.times do |y|
			calcDiamond(x*level, y*level, (x+1)*level, (y+1)*level, entropy, size)
		end
	end
end

def genOceans(size, percent, tolerance)
	sealevel = 0.0
	lower = $min
	upper = $max
	pass = 0
	cPct = 0.0

	puts "\nAttempting to generate a #{(percent*100).to_i}% aquatic world with #{(tolerance*100).to_i}% tolerance"

	while (cPct - percent).abs > tolerance or pass < 1
		pass+=1
		count=0.0

		puts "=PASS #{pass}="
		puts "Generating oceans with sealevel #{sealevel} and bounds [ #{lower} , #{upper} ]"
		
		(0...size).each do |x|
			(0...size).each do |y|
				val = getIndex(x, y)
				if val < sealevel
					$ocean[x][y]=true
					count+=1.0
				else
					$ocean[x][y]=false
				end
			end
		end
		cPct = count / (size**2)
		puts "Produced a #{(cPct*100).to_i}% aquatic world"

		if cPct > percent
			upper = sealevel
			sealevel = lower + ((sealevel - lower) / 2.0)
		elsif cPct < percent
			lower = sealevel
			sealevel = sealevel + ((upper - sealevel) / 2.0) 
		end
	end
	
end

def genImage(size, filename, rscale)
	image = Image.new(size, size)
	offset = -$min
	scale = 255.0 / ($max - $min)
	
	pixels = Array.new(size**2) 
	(0...size).each do |x|
		(0...size).each do |y|
			val = (getIndex(x, y) + offset) * scale
			
			pixel = Pixel.new
			if not $ocean[x][y]
				pixel.red = val.to_i
				pixel.green = val.to_i
			end
			pixel.blue = val.to_i
			pixels[x*size+y]=pixel
		end
	end
	
	image.store_pixels(0,0,size,size,pixels)
	if rscale != 1.0
		image.resize!(size*rscale, size*rscale, filter=BoxFilter, support=1.0)
	end
	image.write(filename)
	image.display
end

$min = $max = 0

entropy     = (ARGV.shift || 100.0).to_f
granularity = (ARGV.shift || 0.5  ).to_f
sz_pwr      = (ARGV.shift || 6    ).to_i
rsz_scale   = (ARGV.shift || 10.0 ).to_f
pct_water   = (ARGV.shift || 0.25 ).to_f
tolerance   = (ARGV.shift || 0.05 ).to_f

max_size=2**sz_pwr + 1
$terrain = mda(max_size, max_size)
$ocean   = mda(max_size, max_size)
genTerrain(2, max_size, entropy, granularity)
genOceans(max_size, pct_water, tolerance)

cTime = Time.now.to_i
genImage(max_size, "#{cTime}.jpeg", rsz_scale)

exit
