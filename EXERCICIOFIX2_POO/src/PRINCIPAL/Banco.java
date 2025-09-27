package PRINCIPAL;
import java.util.Scanner;
import util.Conta;


public class Banco {
	
	public static void main(String[] ags) {

		Scanner sc = new Scanner(System.in); // crio o scanner
		double saldo = 0.0; // declaro saldo 0 para utilizar no if
		
		System.out.println("Ola, bem vindo!");
		System.out.println("Para abrir uma conta primeiro informe, o numero da conta: ");
		int num = sc.nextInt(); //pego valor da conta	
		sc.nextLine();//pula linha
		System.out.println("Qual o nome do titular? ");
		String name = sc.nextLine();//pego nome
		System.out.println("Gostaria de realizar um depósito inicial? y/n");
		String x = sc.nextLine(); //pego a variavel de condição
		
		 
		if(x.equalsIgnoreCase("y")) { //faço a comparação usando o equalIgnoreCase
			//Se digita y(yes) peço o valor do deposito inicial
			System.out.println("Qual o valor inicial? ");
			 saldo = sc.nextDouble();
			 sc.nextLine();
		}
		else{ //uso o equals ignore de novo caso o usuario digite nao
			//E se ele digitou n, informo ao meu programa que o saldo continua 0
			System.out.println("Certo");
			 saldo = 0;	
		}
	
		
	
		Conta conta = new Conta(num, name, saldo);
		
		System.out.println("Dados da conta: " + conta);
		sc.nextLine();
		System.out.println("Qual o valor do deposito? ");
		saldo = sc.nextDouble();
		conta.Deposito(saldo);
		sc.nextLine();
		System.out.println("Atualização" + conta);
		System.out.println("Qual o valor do saque? ");
		saldo = sc.nextDouble();
		conta.Saque(saldo);
		sc.nextLine();
		System.out.println("Atualização" + conta);
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		sc.close();
		
		
		
		
		
		
		
		
	}

}
