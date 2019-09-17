#pragma once

typedef unsigned char uint8;

template<typename T>
class OpaqueVector
{
public:
	OpaqueVector() : m_elementSize(sizeof(T)), m_size(0U), m_capacity(0U), m_data(nullptr) {
	}
	~OpaqueVector() {
		delete[] m_data;
	}

	T& operator[](unsigned int i) {
		T* temp = reinterpret_cast<T*>(m_data + m_elementSize * i);
		return *temp;
	}

	void push_back(const T& element) {
		if (m_size == m_capacity) {
			expand();
		}

		T* newElem = new(m_data + m_elementSize * m_size) T(element);
		m_size++;
	}

	void pop_back() {
		if (m_size > 0) {
			T* elems = new(m_data + m_elementSize * (m_size - 1)) T();
			elems->~T();
			m_size--;
		}
	}

	unsigned int size() const {
		return m_size;
	}

private:
	void expand() {
		m_capacity += 8U;
		uint8* newData = new uint8[m_elementSize * m_capacity];
		if (m_data) {
			memcpy(newData, m_data, m_elementSize * m_size);
			delete[] m_data;
		}
		m_data = newData;
	}

	const size_t m_elementSize;
	size_t m_size;
	size_t m_capacity;
	uint8* m_data;
};