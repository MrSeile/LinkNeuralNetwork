#pragma once

#include <fstream>
#include "Core.h"
#include "Vec2.h"
#include "Neuron.h"
#include "Link.h"

namespace nn
{
	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias = true>
	class LNN_API NeuralNetwork
	{
	private:
		// Stores all the neurons of the net
		std::array<std::vector<Neuron>, tHidden + 2> m_neurons;

		// Stores all the links of the net
		std::array<std::vector<Link>, tHidden + 1> m_links;

		// Stores the structure of the neural network (neurons for layer without bias)
		std::array<uint, tHidden + 2> m_structure;

		// Stores the activation function of the net
		Activation m_activation;

	public:
		// Constructor
		NeuralNetwork(const std::array<uint, tHidden>& hiddenLayers, const Activation& activation, const Vec2<double>& linkInitialValue = { -5, 5 });
		NeuralNetwork(const std::string& path);

		// Calculate the outputs from some inputs
		std::array<double, tOutputs> Calculate(const std::array<double, tInputs>& inputs, bool async = true);
		
		// Adjust the link weights to return the optimal outputs to the inputs
		// Returns the error
		double Train(const std::array<double, tInputs>& inputs, const std::array<double, tOutputs>& optimal, const double& lRate, const double& dropout = 0.0, bool async = true);

		void SaveToFile(const std::string& path);
		bool LoadFromFile(const std::string& path);

		const std::array<uint, tHidden + 2>& GetStructure() const;
		const std::array<std::vector<Neuron>, tHidden + 2>& GetNeurons() const;
		const std::array<std::vector<Link>, tHidden + 1>& GetLinks() const;
	};

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::NeuralNetwork(const std::array<uint, tHidden>& hiddenLayers, const Activation& activation, const Vec2<double>& linkInitalRange)
		: m_activation(activation)
	{
		/////////////////////////
		// Store the structure
		/////////////////////////
		m_structure[0] = tInputs;

		for (uint i = 0; i < tHidden; i++)
		{
			m_structure[i + 1] = hiddenLayers[i];
		}

		m_structure.back() = tOutputs;

		/////////////////////////
		// Create the neurons
		/////////////////////////
		// Inputs
		for (uint i = 0; i < tInputs + tBias; i++)
		{
			m_neurons[0].push_back(Neuron(Activation::none));
		}

		// Hidden
		for (uint i = 0; i < tHidden; i++)
		{
			for (uint j = 0; j < hiddenLayers[i] + tBias; j++)
			{
				m_neurons[i + 1].push_back(Neuron(activation));
			}
		}

		// Outputs
		for (uint i = 0; i < tOutputs; i++)
		{
			m_neurons.back().push_back(Neuron(activation));
		}

		/////////////////////////
		// Create the links
		/////////////////////////
		// Links
		for (uint i = 0; i < tHidden + 1; i++)
		{
			for (uint n1 = 0; n1 < m_structure[i] + tBias; n1++)
			{
				for (uint n2 = 0; n2 < m_structure[i + 1]; n2++)
				{
					m_links[i].push_back(Link(linkInitalRange));
				}
			}
		}
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::NeuralNetwork(const std::string& path)
	{

	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline std::array<double, tOutputs> NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::Calculate(const std::array<double, tInputs>& inputs, bool async)
	{
		// Output array
		std::array<double, tOutputs> out;

		// Set the value of the neurons from the fist layer to the input
		for (uint i = 0; i < tInputs; i++)
		{
			m_neurons[0][i].value = inputs[i];
		}

		//////////////////////////
		// Calclulate the output
		//////////////////////////
		// For each neuron from second layer (skip the input layer)
		for (uint i = 1; i < tHidden + 2; i++)
		{
			//for (uint j = 0; j < m_structure[i]; j++)
			for_async(0, m_structure[i], [&](const int& j)
			{
				double sum = 0;

				// For each link attached to the neuron
				for (uint k = 0; k < m_structure[i - 1] + tBias; k++)
				{
					// link index = previousNeuronIndex * currentLayerSize + currentNeuronIndex
					uint index = k * m_structure[i] + j;

					// 
					sum += m_links[i - 1][index].weight * m_neurons[i - 1][k].value;
				}

				// Pass the value through the activation fucntion
				m_neurons[i][j].value = m_neurons[i][j].activation.funct(sum);

				// Set the raw value to pass it throught the derivative
				m_neurons[i][j].raw = sum;
			}, async);
		}

		// Set the output array to be the last layer of the net
		for (uint i = 0; i < tOutputs; i++)
		{
			out[i] = m_neurons.back()[i].value;
		}

		return out;
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline double NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::Train(const std::array<double, tInputs>& inputs, const std::array<double, tOutputs>& optimal, const double& lRate, const double& dropout, bool async)
	{
		// Make the guess to calclulate the error
		std::array<double, tOutputs> guess = this->Calculate(inputs, async);

		// Dropout
		if (dropout > 0)
		{
			for (uint i = 1; i < tHidden + 1; i++)
			{
				//for (uint j = 0; j < m_structure[i]; j++)
				for_async(0, m_structure[i], [&](const int& j)
				{
					if (randRange(0, 1) < dropout)
					{
						m_neurons[i][j].value = 0;
						m_neurons[i][j].raw = 0;
					}
				}, async);
			}
		}
		
		// Calculate errors
		for (uint i = 0; i < tOutputs; i++)
		{
			m_neurons.back()[i].error = optimal[i] - guess[i];
		}

		for (int i = tHidden; i >= 0; i--)
		{
			//for (int j = 0; j < m_structure[i] + tBias; j++)
			for_async(0, m_structure[i] + tBias, [&](const int& j)
			{
				m_neurons[i][j].error = 0;

				for (uint k = 0; k < m_structure[i + 1]; k++)
				{
					uint index = j * m_structure[i + 1] + k;

					// Dont quite understand :(
					m_neurons[i][j].error += m_links[i][index].weight * m_neurons[i + 1][k].error;
				}
			}, async);
		}

		// Adjust the weights
		for (uint i = 0; i < tHidden + 1; i++)
		{
			//for (uint j = 0; j < m_structure[i] + tBias; j++)
			for_async(0, m_structure[i] + tBias, [&](const int& j)
			{
				for (uint k = 0; k < m_structure[i + 1]; k++)
				{
					uint index = j * m_structure[i + 1] + k;

					// Dont understand ( f'(frontNeuron.value) * frontNeuron.error * backNeuron.value * learningRate )
					double delta = m_neurons[i + 1][k].activation.derivate(m_neurons[i + 1][k].raw) * m_neurons[i + 1][k].error * m_neurons[i][j].value * lRate;

					m_links[i][index].weight += delta;
				}
			}, async);
		}

		double err = 0;
		for (const Neuron& x : m_neurons.back())
		{
			err += abs(x.error);
		}

		return err / tOutputs;
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline void NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::SaveToFile(const std::string& path)
	{
		std::fstream file;
		file.open(path, std::fstream::out | std::fstream::trunc);
	
		file << tBias << "\n";
		file << tInputs << "\n";
		file << tOutputs << "\n";
		file << tHidden << "\n";

		for (uint i = 1; i < tHidden + 1; i++)
		{
			file << m_structure[i] << "\n";
		}

		for (uint i = 0; i < tHidden + 1; i++)
		{
			for (uint j = 0; j < m_links[i].size(); j++)
			{
				file << m_links[i][j].weight << "\n";
			}
		}

		file.close();
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	inline bool NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::LoadFromFile(const std::string& path)
	{
		std::cout << "Loading Neural Network from file...\n";

		std::fstream file;
		file.open(path);

		std::vector<std::string> save = FileToArray(file);
		
		if ((bool)std::stoi(save[0]) != tBias)
		{
			std::cout << "Failed to load: Neural Network structure do not match\n\n";
			return false;
		}
		if (std::stoi(save[1]) != tInputs)
		{
			std::cout << "Failed to load: Neural Network structure do not match\n\n";
			return false;
		}
		if (std::stoi(save[2]) != tOutputs)
		{
			std::cout << "Failed to load: Neural Network structure do not match\n\n";
			return false;
		}
		if (std::stoi(save[3]) != tHidden)
		{
			std::cout << "Failed to load: Neural Network structure do not match\n\n";
			return false;
		}
		for (uint i = 1; i < tHidden + 1; i++)
		{
			if (m_structure[i] != std::stoi(save[i + 3]))
			{
				std::cout << "Failed to load: Neural Network structure do not match\n\n";
				return false;
			}
		}

		uint index = 0;
		for (uint i = 0; i < tHidden + 1; i++)
		{
			for (uint j = 0; j < m_links[i].size(); j++)
			{
				m_links[i][j].weight = std::stod(save[index + 3 + tHidden + 1]);
				index++;
			}
		}

		std::cout << "Loaded!\n\n";

		return true;
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	const std::array<uint, tHidden + 2>& NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::GetStructure() const
	{
		return m_structure;
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	const std::array<std::vector<Neuron>, tHidden + 2>& NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::GetNeurons() const
	{
		return m_neurons;
	}

	template<uint tInputs, uint tHidden, uint tOutputs, bool tBias>
	const std::array<std::vector<Link>, tHidden + 1>& NeuralNetwork<tInputs, tHidden, tOutputs, tBias>::GetLinks() const
	{
		return m_links;
	}
}