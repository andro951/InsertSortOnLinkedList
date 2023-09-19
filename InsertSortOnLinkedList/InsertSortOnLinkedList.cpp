/*
Author - Isaac Richards
Date 18SEP23
Description - Custom linkedList with sorting using insert sort.
*/

#include <iostream>
#include <string>
#include <functional>
#include <memory>

template<typename T>
class linkedList;

/// <summary>
/// element acts as a container for a value in a linkedList that stores the value and pointers to the surrounding elements and the list.
/// </summary>
template<typename T>
struct element {
	element(const element& other) = delete;
public:
	T value;
	element<T>* nextElement = nullptr;
	element<T>* previousElement = nullptr;
	linkedList<T>* list;

	template<typename... Args>
	element(linkedList<T>* List, Args&&... args) : list(List), value(std::forward<Args>(args)...) {}

	~element() {
		BridgeAcross();
		list->count--;
	}

	//element(const element& other) {
	//	nextElement = other.nextElement;
	//	previousElement = other.previousElement;
	//	value = other.value;
	//	list = other.list;
	//}

	T& Next() const {
		return nextElement->value;
	}

	T& Prev() const {
		return previousElement->value;
	}

	bool IsFirst() const {
		return previousElement == nullptr;
	}

	bool IsEnd() const {
		return nextElement == nullptr;
	}

	bool IsLast() const {
		return !IsEnd() && nextElement->IsEnd();
	}

	/// <summary>
	/// Inserts value just before this element in the list
	/// </summary>
	void InsertNewBeforeMe(const T& value) {
		element<T>* newElement = new element<T>(list, value);
		newElement->InsertMeBeforeOther(this);
	}

	/// <summary>
	/// Emplace value just before this element in the list
	/// </summary>
	template<typename... Args>
	void EmplaceNewBeforeMe(Args&&... args) {
		element<T>* newElement = new element<T>(list, args...);
		newElement->InsertMeBeforeOther(this);
	}

	/// <summary>
	/// Inserts value just after this element in the list
	/// </summary>
	void InsertNewAfterMe(const T& value) {
		element<T>* newElement = new element<T>(list, value);
		newElement->InsertMeAfterOther(this);
	}

	/// <summary>
	/// Emplace value after before this element in the list
	/// </summary>
	template<typename... Args>
	void EmplaceNewAfterMe(Args&&... args) {
		element<T>* newElement = new element<T>(list, args...);
		newElement->InsertMeAfterOther(this);
	}

	void Remove() {
		delete this;
	}

	void MoveMeBeforeOther(element<T>* other) {
		BridgeAcross();
		InsertMeBeforeOther(other);
	}

	void MoveMeAfterOther(element<T>* other) {
		BridgeAcross();
		InsertMeAfterOther(other);
	}

	void Swap(element<T>& other) {
		//Store the surrounding elements of this element
		element<T>* tempPrevious = previousElement;
		element<T>* tempNext = nextElement;

		//Connect this Element to the surrounding elements of other
		previousElement = other.previousElement;
		nextElement = other.nextElement;

		//Connect other to the surrounding elements of this element
		other.previousElement = tempPrevious;
		other.nextElement = tempNext;
	}

	/// <summary>
	/// Connects this element to the surrounding elements, then connects the surrounding elements to this element.
	/// Used when inserting or moving an element.
	/// </summary>
	void InsertMeBeforeOther(element<T>* other) {
		if (other->IsFirst()) {
			//other is the first element

			//Connect other and the list to this element
			previousElement = nullptr;
			nextElement = other;

			//Connect this element to the list and other
			list->firstElement = this;
			other->previousElement = this;
		}
		else {
			//other is not the first element

			//Connect surrounding elements to this element
			previousElement = other->previousElement;
			nextElement = other;

			//Connect this element to surrounding elements
			other->previousElement = this;
			previousElement->nextElement = this;
		}
	}

	/// <summary>
	/// Connects this element to the surrounding elements, then connects the surrounding elements to this element.
	/// Used when inserting or moving an element.
	/// </summary>
	void InsertMeAfterOther(element<T>* other) {
		if (other->IsEnd()) {
			//Other is the end element, not allowed to move the end element, so insert before it instead.
			InsertMeBeforeOther(other);
		}
		else {
			//other is not the end element

			//Connect surrounding elements to this element
			previousElement = other;
			nextElement = other->nextElement;

			//Connect this element to surrounding elements
			other->nextElement = this;
			nextElement->previousElement = this;
		}
	}

	/// <summary>
	/// Connects the previous and next elements from the surrounding elements together.
	/// Used when removing or moving an element.
	/// </summary>
	void BridgeAcross() {
		if (IsFirst()) {
			//This element is the first element

			//Connect the list to the next element
			list->firstElement = nextElement;
			nextElement->previousElement = nullptr;
		}
		else {
			//This element is not the first element

			//Connect the previous and next elements together
			previousElement->nextElement = nextElement;
			nextElement->previousElement = previousElement;
		}
	}

public:
	bool operator<(const element<T>& other) const {
		return value < other.value;
	}

	bool operator>(const element<T>& other) const {
		return value > other.value;
	}

	bool operator<=(const element<T>& other) const {
		return value <= other.value;
	}

	bool operator>=(const element<T>& other) const {
		return value >= other.value;
	}

	bool operator==(const element<T>& other) const {
		return value == other.value;
	}

	bool operator!=(const element<T>& other) const {
		return value != other.value;
	}

	template<typename T>
	static void Inc(element<T>*& el) {
		el = el->nextElement;
	}

	template<typename T>
	static void Dec(element<T>*& el) {
		el = el->previousElement;
	}
};

/// <summary>
/// linkedList uses elements of "element" to store values.
/// </summary>
template<typename T>
class linkedList {
	/// <summary>
	/// First element of the list which starts as the same as the end element and is changed when the 
	///		first element is added or when elements are removed/inserted.
	/// </summary>
	element<T>* firstElement;

	/// <summary>
	/// End element is a dummy element to help with knowing where the end is and inserting new elements.<br/>
	/// It should always be the end element.
	/// </summary>
	element<T>* endElement;

	/// <summary>
	/// Number of elements in the list, not including the end element.
	/// </summary>
	int count = 0;

	/// <summary>
	/// If sorted is true, adding elements will automatically sort them.
	/// Otherwise, they will be added to the end of the list.
	/// </summary>
	bool sorted = false;

	typedef std::string(*ToStringFunc)(const T&);
	static std::string ElementToStringFuncDefault(const T& value) { return ""; }
	ToStringFunc elementToStringFunc;

	/// <summary>
	/// Creates the 
	/// </summary>
	void Setup() {
		endElement = new element<T>(this);
		firstElement = endElement;
	}

	linkedList(const linkedList& other) = delete;

	//linkedList(const linkedList& other) {
	//	//Fatal design flaw, each element tracks the list, so you need to update the list pointer for each element.
	//	element<T>* current = First();
	//	while (current != nullptr) {
	//		current->list = this;
	//		element<T>::Inc(current);
	//	}
	//}

	friend struct element<T>;

public:
	friend struct element<T>;

	/// <summary>
	/// Initialize the list with no values except the end element.
	/// </summary>
	/// <param name="sort"></param>
	linkedList(ToStringFunc ElementToStringFunc = ElementToStringFuncDefault, bool sort = false) : elementToStringFunc(ElementToStringFunc), sorted(sort) {
		Setup();
	}

	/// <summary>
	/// Initialize the list with value as the first value.
	/// </summary>
	linkedList(const T& value, ToStringFunc ElementToStringFunc = ElementToStringFuncDefault, bool sort = false) : elementToStringFunc(ElementToStringFunc), sorted(sort) {
		Setup();
		Add(value);
	}

	/// <summary>
	/// Initialize the list, then add all values from the array to the list.
	/// </summary>
	template<size_t S>
	linkedList(const T(&arr)[S], ToStringFunc ElementToStringFunc = ElementToStringFuncDefault, bool sort = false) : elementToStringFunc(ElementToStringFunc), sorted(sort) {
		Setup();
		Add(arr);
	}

	linkedList(linkedList&& other) noexcept
		: firstElement(other.firstElement), endElement(other.endElement), count(other.count), sorted(other.sorted), elementToStringFunc(other.elementToStringFunc) {
		// Reset the source list to a valid state
		other.firstElement = nullptr;
		other.endElement = nullptr;
		other.count = 0;
		other.sorted = false;
		other.elementToStringFunc = nullptr;
	}

	linkedList& operator=(const linkedList& other) {
		//Fatal design flaw, each element tracks the list, so you need to update the list pointer for each element.
		element<T>* current = First();
		while (current != nullptr) {
			current->list = this;
			element<T>::Inc(current);
		}
	}

	/// <summary>
	/// Gets the first element in the list.
	/// </summary>
	element<T>* First() const {
		return firstElement;
	}

	/// <summary>
	/// Gets the last element in the list.  Equivalent of count - 1 if indexing.
	/// </summary>
	element<T>* Last() const {
		return endElement->previousElement;
	}

	/// <summary>
	/// Gets the end element.
	/// The endElelment should only be used for checking if the end of the list has been reached or inserting elements before it.
	/// </summary>
	element<T>* End() const {
		return endElement;
	}

	/// <summary>
	/// Gets the number of elements in the list, not including the end element.
	/// </summary>
	int Count() {
		return count;
	}

	/// <summary>
	/// Creates a new element, and adds it to the list.
	/// </summary>
	void Add(const T value) {
		if (sorted) {
			//If the list is already sorted, insert sort the new element.
			InsertSort(value);
		}
		else {
			//If not sorted, add the new element to the end of the list.
			endElement->InsertNewBeforeMe(value);
		}

		++count;
	}

	/// <summary>
	/// Creates a new element by directly constructing it's value in place, and adds it to the list.
	/// </summary>
	template<typename... Args>
	void Emplace(Args&&... args) {
		element<T>* newElement = new element<T>(this, args...);
		if (sorted) {
			//If the list is already sorted, insert sort the new element.
			InsertSort(newElement);
		}
		else {
			//If not sorted, add the new element to the end of the list.
			newElement->InsertMeBeforeOther(endElement);
		}

		++count;
	}

	/// <summary>
	/// Inserts the value into the list in sorted order.
	/// </summary>
	void InsertSort(const T& value) {
		element<T>* current = FindInsertElement(value);
		current->InsertNewBeforeMe(value);
	}

	/// <summary>
	/// Creates a new element by directly constructing it's value in place, and inserts it into the list in sorted order.
	/// </summary>
	template<typename... Args>
	void EmplaceSort(Args&&... args) {
		element<T>* newElement = new element<T>(this, args...);
		InsertSort(newElement);
	}

	/// <summary>
	/// Inserts the new element into the list in sorted order.
	/// </summary>
	void InsertSort(element<T>* newElement) {
		element<T>* current = FindInsertElement(newElement->value);
		newElement->InsertMeBeforeOther(current);
	}

	/// <summary>
	/// Adds all values from the array to the list from [start, end).
	/// </summary>
	/// <param name="end">Stops adding elements when start < end.  The value arr[end] is not added.</param>
	template<size_t S>
	void Add(const T(&arr)[S], int start = 0, int end = S) {
		if (end <= start)
			return;

		//Add each value from [start, end).
		while (start < end) {
			Add(arr[start++]);
		}
	}

	/// <summary>
	/// Deletes all elements in the list, except the end element.
	/// </summary>
	void Clear() {
		element<T>* current = End();
		while (!current->IsFirst()) {
			current->previousElement->Remove();
		}
	}

	/// <summary>
	/// Finds the element where the new value/element would be inserted.
	/// Should be used when adding new elements to a sorted list.
	/// </summary>
	/// <param name="after">Only used when values in the list are equal to the value being searched.
	/// If true, the resulting element will occur just after an element with the same value.
	/// If false, it will be before the last element of the same value.</param>
	element<T>* FindInsertElement(const T& value, bool after = true) {
		element<T>* startElement = First();
		return FindInsertElement(startElement, value, after);
	}

	/// <summary>
	/// Finds the element where the new value/element would be inserted.
	/// </summary>
	/// <param name="after">- Only matters if there are duplicates in a list or if value is equal to an element in the list.
	/// after being true will continue to pass elements of the same value, putting it after the existing ones.
	/// after being false will stop at the first element of the same value, putting it before the existing ones.</param>
	/// <param name="goingUp">- true means it will go up the list, false means it will go down the list.</param>
	/// <returns></returns>
	element<T>* FindInsertElement(element<T>* start, const T& value, bool after = true) {
		while (!start->IsLast() && (after && value >= start->value || !after && value > start->value)) {
			element<T>::Inc(start);
		}

		return start;
	}

	/// <summary>
	/// Finds the element where the new value would be inserted by going backwards through the list.
	/// </summary>
	/// <param name="after">- Only matters if there are duplicates in a list or if value is equal to an element in the list.
	/// after being true will continue to pass elements of the same value, putting it after the existing ones.
	/// after being false will stop at the first element of the same value, putting it before the existing ones.</param>
	element<T>* FindInsertElementReverse(element<T>* start, const T& value, bool after = true) {
		while (!start->IsFirst() && (after && value < start->Prev() || !after && value <= start->Prev())) {
			element<T>::Dec(start);
		}

		return start;
	}

	/// <summary>
	/// Checks if the list contains the value.
	/// </summary>
	bool Contains(const T& value) {
		element<T>* current = FindInsertElement(value, false);
		return current->value == value;
	}

	/// <summary>
	/// Inserts a new element into the list at the current element.
	/// </summary>
	void Insert(element<T>* current, const T& value, bool after = false) {
		if (!after) {
			current->InsertNewBeforeMe(value);
		}
		else {
			current->InsertNewAfterMe(value);
		}
	}

	/// <summary>
	/// Removes the current element from the list.
	/// </summary>
	void Remove(element<T>* current) {
		current->Remove();
	}

	/// <summary>
	/// Removes the element at the index.
	/// </summary>
	void RemoveAt(int index) {
		Remove((*this)[index]);
	}

	/// <summary>
	/// Gets the value by index.  Should only be used when no elements are already available to use to move through the list.
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	element<T>* operator[](int index) {
		if (index < 0 || index >= count)
			throw std::out_of_range("Index out of range. index: " + std::to_string(index) + ", count: " + std::to_string(count));

		if (index <= count / 2) {
			element<T>* current = First();
			while (index-- > 0) {
				element<T>::Inc(current);
			}

			return current;
		}
		else {
			element<T>* current = End();
			while (index++ < count) {
				element<T>::Dec(current);
			}

			return current;
		}
	}

	/// <summary>
	/// Sorts the list using insertion sort.
	/// </summary>
	void Sort() {
		//Once sorted, it will stay sorted.
		if (sorted)
			return;

		sorted = true;

		element<T>* current = First();
		element<T>::Inc(current);
		while (!current->IsEnd()) {
			element<T>* insertElement = FindInsertElementReverse(current, current->value);
			element<T>* copy = current;
			element<T>::Inc(current);
			if (insertElement == copy)
				continue;//Already in the right place.

			copy->MoveMeBeforeOther(insertElement);
		}
	}

	/// <summary>
	/// Converts the list to a string.
	/// </summary>
	/// <param name="label">- label is printed before the string if included.</param>
	/// <param name="reverse">- For testing to make sure the list can be traversed backwards.</param>
	std::string ToString(std::string label = "", bool reverse = false) const {
		bool first = true;
		std::string result = "";
		if (label != "") {
			result += label;
			if (reverse)
				result += "-Reverse";

			result += ": ";
		}

		result += "{ ";

		if (!reverse) {
			for (element<T>* current = firstElement; current->nextElement != nullptr; current = current->nextElement) {
				if (first) {
					first = false;
				}
				else {
					result += ", ";
				}

				result += elementToStringFunc(current->value);
			}
		}
		else {
			for (element<T>* current = endElement->previousElement; current != nullptr; current = current->previousElement) {
				if (first) {
					first = false;
				}
				else {
					result += ", ";
				}

				result += elementToStringFunc(current->value);
			}
		}

		result += " }";

		return result;
	}

	static std::string ToString(const linkedList<T>& list) {
		return list.ToString();
	}

	/// <summary>
	/// Prints the list to the console.
	/// </summary>
	void Print(std::string label = "", bool reverse = false) const {
		std::cout << ToString(label, reverse) << std::endl;
	}

	bool operator==(const linkedList& other) const {
		if (count != other.count)
			return false;

		element<T>* current = firstElement;
		element<T>* otherCurrent = other.firstElement;
		for (; current->nextElement != nullptr && otherCurrent->nextElement != nullptr;
			current = current->nextElement, otherCurrent = otherCurrent->nextElement) {
			if (current->value != otherCurrent->value)
				return false;
		}

		return true;
	}

	bool operator!=(const linkedList& other) const {
		return !(*this == other);
	}

	bool operator<(const linkedList& other) const {
		element<T>* current = firstElement;
		element<T>* otherCurrent = other.firstElement;
		for (; current != nullptr; current = current->nextElement, otherCurrent = otherCurrent->nextElement) {
			if (otherCurrent == nullptr)
				return false;

			if (current->value < otherCurrent->value)
				return true;

			else if (current->value > otherCurrent->value)
				return false;
		}

		return otherCurrent != nullptr;
	}

	bool operator>(const linkedList& other) const {
		element<T>* current = firstElement;
		element<T>* otherCurrent = other.firstElement;
		for (; current != nullptr; current = current->nextElement, otherCurrent = otherCurrent->nextElement) {
			if (otherCurrent == nullptr)
				return true;

			if (current->value > otherCurrent->value)
				return true;

			else if (current->value < otherCurrent->value)
				return false;
		}

		return false;
	}

	bool operator<=(const linkedList& other) const {
		return !(*this > other);
	}

	bool operator>=(const linkedList& other) const {
		return !(*this < other);
	}
};

template<typename T>
std::string stdToString(const T& value) {
	return std::to_string(value);
}

int main() {
	float arr[] = { 23, 54, 2, 8, 3.2, 14, 43, 0, 9, 2 };

	linkedList<float> list = linkedList<float>(arr, stdToString);

	list.Print("list");
	list.Sort();
	list.Print("Sorted list");

	list.Clear();
}
//Sort = (n - 2) * (FindInsertIndex / 2) + 1 + MoveToBefore + 2x [i]
	//FindInsertIndex = (log2(n) - 1) * (6 + 2 * [i])
	//MoveToBefore = 8
	//[i] = 3/4 * (n + 4)

	//Sort = (n - 2) * (log2(n) - 1) * (6 + 2 * 3/4 * (n + 4)) / 2 + 1 + 8 + 2 * 3/4 * (n + 4)
	//     = (n - 2) * (log2(n) - 1) * (6 + 3/2 * n + 6) / 2 + 9 + 3/2 * n + 6
	//	   = (n - 2) * (log2(n) - 1) * (3/4 * n + 6) + 3/2 * n + 15
	//	   ~ n * log2(n) * n + n
	//	   ~ n^2 * log2(n)
	//This is so inefficient because I chose to use the same logic (mainly indexing the list) instead of updating it to something more efficient.
	//If the linear search is more efficient for a linked list because indexing them costs as much as the index number compared to 1 for arrays.
	//If the indexing were replaced, it would bring the n down to ~log(n), so the final result would be n * log(n)^2
