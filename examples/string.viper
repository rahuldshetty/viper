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
