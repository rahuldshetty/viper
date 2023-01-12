# String <!-- {docsify-ignore-all} -->

String is a sequence of character enclosed within double quotes.

- Strings are mutable by nature.
- You can only update a single character at once in the string.
- You can use the range selector operator [start:end] to extract a substring. This will return another string object starting at index "start" and ending at index "end-1". (Convention similar to Python).

## Example

```string.viper
// Program to check whether string is Palindrome

test_success_string = "saippuakivikauppias"
test_failure_string = "abracadabra"

fn isPalindrome(string){
    n = len(string)

    for( i=0; i < n / 2 + 1; i = i + 1){
        if(string[i] != string[n - i - 1]){
            return false
        }
    }
    return true
}

fn testPalindrome(string){
    if isPalindrome(string){
        print "Given string '" + string + "' is Palindromic."
    } else {
        print "Given string '" + string + "' is NOT Palindromic."
    }
}

testPalindrome(test_success_string)
testPalindrome(test_failure_string)

```

Output:
```
Given string 'saippuakivikauppias' is Palindromic.
Given string 'abracadabra' is NOT Palindromic.
```

## Note

- Viper supports [String Interning](https://en.wikipedia.org/wiki/String_interning) to store only one copy of each distinct string value. This makes comparison of string equality faster.
- Characters in a Viper strings are limited to ASCII based characters. (Unicode and more to follow).
- Currently doesn't have support for escape characters. (TODO)
