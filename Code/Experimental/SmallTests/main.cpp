#include <iostream>

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Random/Random.h"

using namespace CKE;

class MoveOnly
{
public:
	MoveOnly(String name) {
		std::cout << "Constructor Called: " << name << "\n";
		m_Value = Random::I32();
		m_Name = name;
	};
	MoveOnly(const MoveOnly& other) {
		m_Name = other.m_Name;
		m_Value = other.m_Value;
		std::cout << "Copy Called: " << m_Name << "\n";
	};
	MoveOnly(MoveOnly&& other) noexcept {
		m_Value = other.m_Value;
		m_Name = other.m_Name;
		std::cout << "Move Called: " << m_Name << "\n";
	}
	MoveOnly& operator=(const MoveOnly& other) {
		m_Name = other.m_Name;
		m_Value = other.m_Value;
		std::cout << "Copy Equal Called: " << m_Name << "\n";
	};
	MoveOnly& operator=(MoveOnly&& other) noexcept {
		m_Value = other.m_Value;
		m_Name = other.m_Name;
		other.m_Value = 0.0f;
		std::cout << "Move Equal Called: " << m_Name << "\n";
	}

	~MoveOnly() {
		std::cout << "Destructor Called: " << m_Name << "\n";
		if(m_Value == 0) {
			std::cout << "ALERT LEAK: " << m_Name << "\n";
		}
	}

	i32 GetValue() const {
		return m_Value;
	}

private:
	i32 m_Value;
	String m_Name;
};

int main()
{
	Vector<MoveOnly> m;
	m.push_back(MoveOnly{"A"});
	m.emplace_back(MoveOnly{"B"});
	m.push_back(MoveOnly{"C"});
	m.emplace_back(MoveOnly{"D"});

	for (MoveOnly const& moveOnly : m) {
		std::cout << moveOnly.GetValue() << "\n";
	}

	return 0;
}
