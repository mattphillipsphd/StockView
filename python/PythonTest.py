import sys

def main():
    if len(sys.argv) < 3:
        print("Usage: script.py <num1> <num2>")
        return

    try:
        num1 = float(sys.argv[1])
        num2 = float(sys.argv[2])
        result = num1 + num2
        print(f"The sum of {num1} and {num2} is {result}")
    except ValueError:
        print("Please provide two valid numbers.")

if __name__ == "__main__":
    main()
