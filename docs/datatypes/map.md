# Map <!-- {docsify-ignore-all} -->

Map data type is used to store elements that have key:value pair relationship. 
Like other programming languages, in Viper we denote Map in curly braces - {} and enclose a comma separated list of key:value pairs.

- Maps are mutable.
- Each key in the map is unique.
- Map keys must be [string](/string.md) or [number](/number.md) data type constants.
- You can store any data type as a map value.
- When assigning values to a key, it will automatically create or update entry based on the key availability.

## Example

```map.viper
// Create a new map
capitals = {"USA":"Washington DC", "France":"Paris", "India":"New Delhi"}

// Get map entry
print "Capital of USA:" + capitals["USA"]

print "Capital of India:" + capitals["India"]

// Set map entry
capitals["Japan"] = "Tokyo"
print "Capital of Japan:" + capitals["Japan"]

print "Map: " + str(capitals)

```

Output
```
Capital of USA:Washington DC
Capital of India:New Delhi
Capital of Japan:Tokyo
Map: {USA:Washington DC, France:Paris, Japan:Tokyo, India:New Delhi}
```

## Note

- If a key doesn't exist in the Map then the interpreter errors out. (TODO: Implementation of Try/Catch and Error Objects)
- Map uses an internal hash function to find the right location to perform insertion and search. If location is occupied then it performs [Linear Probing](https://en.wikipedia.org/wiki/Linear_probing) to find the next best location for the task.
