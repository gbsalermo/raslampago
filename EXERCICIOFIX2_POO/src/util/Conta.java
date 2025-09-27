package util;

public class Conta {
	
	private int num; // numero da conta
	public String name; // nome do titular
	//private double dpi; //Deposito inicial
	private double saldo; //Saldo 
	
	public Conta() { //construtor padrão
		
	}

	public Conta(int num, String name, double saldo) { //Construtor para cada variavel
		this.num = num;
		this.name = name;
		this.saldo = saldo;
	}

	//Encapsulamento
	public String getName() { 
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
	
	public double SaldoTotal(double saldo) {
		return saldo;
	}
	

	public void Deposito(double saldo) { 
		this.saldo += saldo; 
	}
	public void Saque(double saldo) { 
		
		this.saldo -= saldo + 5; 
	}

	public String toString() {
		return String.format("Conta %d, Titular: %s, Saldo: %.2f", num, name, saldo);
	}
}
